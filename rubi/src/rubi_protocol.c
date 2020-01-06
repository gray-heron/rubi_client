
#include "rubi.h"

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// ========== INIT ==========

typedef enum
{
    no_init,
    preticket_holdoff,
    ticket_sent,
    postticket_holdoff,
    enumerating_capabilities,
    inited
} init_stage_t;

static uint8_t init_stage = no_init;
uint32_t       holdoff_timer;

// ========== MESSAGE SEGMENTATION ==========

static int32_t rubi_rx_cursor = 0;
static uint8_t rubi_tx_buffer[RUBI_BUFFER_SIZE];
static uint8_t rubi_rx_buffer[RUBI_BUFFER_SIZE];
static int32_t rubi_tx_cursor_low;
static int32_t rubi_tx_cursor_high;

static rubi_dataheader rubi_tx_current_header = {0, 0, 0, 0};

static int32_t blocks_sent;
static int32_t block_transfer;
static int32_t blocks_received;

// ========== INTERFACE ==========

uint16_t rubi_lottery_addr;
uint16_t rubi_nodeid;

extern uint8_t rubi_state;
extern uint8_t rubi_wake;
extern uint8_t rubi_ready_for_finalization;

extern uint32_t rubi_last_keepalive_time;

// ========== INTERNAL ==========

static void rubi_wait_for_tx_and_flock(int32_t);
static void rubi_tx_enqueue_back(uint8_t* data, int32_t size);

extern void rubi_write_ffdata(uint8_t id, uint8_t* ptr);
extern void rubi_keepalive_received();

// ========== IMPLEMENTATION ==========

uint8_t* rubi_get_tx_chunk(uint8_t size)
{
    static uint8_t buf[64];

    if((uint32_t)rubi_tx_cursor_high + size > RUBI_BUFFER_SIZE)
    {
        memcpy(
            buf, &rubi_tx_buffer[rubi_tx_cursor_high],
            RUBI_BUFFER_SIZE - rubi_tx_cursor_high);
        memcpy(
            &buf[RUBI_BUFFER_SIZE - rubi_tx_cursor_high], rubi_tx_buffer,
            size - (RUBI_BUFFER_SIZE - rubi_tx_cursor_high));

        return buf;
    }

    return &rubi_tx_buffer[rubi_tx_cursor_high];
}

void rubi_protocol_init(void)
{
    rubi_can_add_filter(RUBI_BROADCAST1, 0x7ff, 0,
                        3); // interupt broadcast

    rubi_attempt_handshake();
}

void rubi_attempt_handshake()
{
    // fixme: add proper filter disablingb
    rubi_can_add_filter(0, 0x7ff, 0, 1); // interrupt d1irect
    rubi_can_add_filter(0, 0x7ff, 1, 2); // poll direct

    rubi_state    = RUBI_STATE_INITING;
    holdoff_timer = rubi_rng() % RUBI_ARBITRATION_TIME0;
    init_stage    = preticket_holdoff;
}

void rubi_continue_init()
{
    uint16_t protocol_version = RUBI_PROTOCOL_VERSION;

    switch(init_stage)
    {
    case preticket_holdoff:
        if(!holdoff_timer)
        {
            rubi_lottery_addr =
                rubi_rng() % (RUBI_LOTTERY_RANGE_HIGH - RUBI_LOTTERY_RANGE_LOW);

            rubi_can_add_filter(
                RUBI_LOTTERY_RANGE_LOW + rubi_lottery_addr, 0x7ff, 0,
                2); // interupt broadcast

            rubi_dataheader h = {RUBI_LOTTERY_RANGE_LOW + rubi_lottery_addr,
                                 RUBI_MSG_LOTTERY, 0, 2};

            rubi_wait_for_tx_and_flock(sizeof(protocol_version));
            rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
            rubi_tx_enqueue_back(
                (uint8_t*)&protocol_version, sizeof(protocol_version));
            rubi_funlock();
            rubi_event_continue_tx();

            init_stage    = ticket_sent;
            holdoff_timer = rubi_rng() % RUBI_ARBITRATION_TIME1;
        }
        else
        {
            --holdoff_timer;
        }
        break;

    case ticket_sent:
        // do nothing, state will be changed when processing server's lottery
        // rensponce
        break;

    case postticket_holdoff:
        if(!holdoff_timer)
        {
            rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid,
                                 RUBI_MSG_LOTTERY, 0, 0};
            rubi_wait_for_tx_and_flock(0);
            rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));

            rubi_funlock();
            rubi_event_continue_tx();

            rubi_capabilities_enumeration_start();
            rubi_event_continue_tx();
            rubi_last_keepalive_time = rubi_time_sec;
            init_stage               = enumerating_capabilities;
        }
        else
        {
            --holdoff_timer;
        }
        break;

    case enumerating_capabilities:
        if(!rubi_capabilities_enumeration_continue())
        {
            rubi_send_empty(RUBI_MSG_INIT_COMPLETE);
            init_stage = inited;
        }
        rubi_event_continue_tx();

        break;

    case inited:
        break;

    case no_init:
        break;

    default:
        RUBI_ASSERT(false);
    }
}

void rubi_lottery_inbound(uint8_t addr_in)
{
    RUBI_ASSERT(rubi_state == RUBI_STATE_INITING && init_stage == ticket_sent)
    rubi_nodeid = addr_in;

    rubi_can_add_filter(
        RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, 0x7ff, 0,
        1); // interrupt direct
    rubi_can_add_filter(
        RUBI_ADDRESS_RANGE2_LOW + rubi_nodeid, 0x7ff, 1,
        2); // poll direct

    init_stage = postticket_holdoff;
}

void rubi_event_inbound(rubi_can_msg_t rx)
{
    if(rx.id >= RUBI_LOTTERY_RANGE_LOW && rx.id <= RUBI_LOTTERY_RANGE_HIGH)
    {
        if(rx.id - RUBI_LOTTERY_RANGE_LOW != rubi_lottery_addr)
            return;

        if(rx.data_length != 1)
            return;

        rubi_lottery_inbound(rx.data[0]);
    }

    else if(
        (rx.id >= RUBI_ADDRESS_RANGE1_LOW &&
         rx.id <= RUBI_ADDRESS_RANGE1_HIGH) ||
        rx.id == RUBI_BROADCAST1)
    {
        uint8_t* potential_data_ptr = (uint8_t*)&rx.data[2];
        RUBI_ASSERT(rx.data_length >= 1);

        switch(rx.data[0] & RUBI_MSG_MASK)
        {
        case RUBI_MSG_FIELD:
        case RUBI_MSG_FUNCTION:
            RUBI_ASSERT(rx.data_length >= 2);

            if(rx.data[0] & RUBI_FLAG_BLOCK_TRANSFER)
            {
                RUBI_ASSERT(rx.data[2] == blocks_received);
                blocks_received    = 0;
                potential_data_ptr = rubi_rx_buffer;
                rubi_rx_cursor     = 0;
            }

            rubi_write_ffdata(rx.data[1], potential_data_ptr);

            break;

        case RUBI_MSG_BLOCK:
            RUBI_ASSERT(rx.data_length > 1);
            uint32_t data_len = rx.data_length - 1;

            memcpy(&rubi_rx_buffer[rubi_rx_cursor], &rx.data[1], data_len);
            rubi_rx_cursor += data_len;
            blocks_received += 1;
            break;

        case RUBI_MSG_COMMAND:
            RUBI_ASSERT(rx.data_length == 2);

            switch((uint8_t)rx.data[1])
            {
            case RUBI_COMMAND_LOTERRY:
                rubi_protocol_init();
                break;
            case RUBI_COMMAND_HARDSLEEP:
                rubi_die();
                break;
            case RUBI_COMMAND_SOFTSLEEP:
                rubi_sleep();
                break;
            case RUBI_COMMAND_WAKE:
                rubi_wakeup();
                break;
            case RUBI_COMMAND_OPERATIONAL:
                rubi_state = RUBI_STATE_OPERATIONAL;
                rubi_callback_operational();
                break;
            case RUBI_COMMAND_HOLD:
                rubi_state = RUBI_STATE_HOLD;
                break;
            case RUBI_COMMAND_KEEPALIVE:
                rubi_send_command(
                    RUBI_COMMAND_KEEPALIVE, &rubi_wake, sizeof(rubi_wake));
                rubi_keepalive_received();
                break;
            case RUBI_COMMAND_REBOOT:
                rubi_callback_lost();
                rubi_attempt_handshake();
                break;
            }

            break;
        default:
            RUBI_ASSERT(0);
        }
    }
}

// NOT counting the header
int32_t rubi_tx_avaliable_space(void)
{
    if(rubi_tx_cursor_low == -1)
        return 0;

    if(rubi_tx_cursor_high > rubi_tx_cursor_low)
    {
        return rubi_tx_cursor_high - rubi_tx_cursor_low;
    }

    return RUBI_BUFFER_SIZE - rubi_tx_cursor_low + rubi_tx_cursor_high;
}

void rubi_tx_checkfull()
{
    if(rubi_tx_cursor_high == rubi_tx_cursor_low)
        rubi_tx_cursor_low = -1;
}

// WARNING! this assumes the data will fit
void rubi_tx_enqueue_back(uint8_t* data, int32_t size)
{
    if(!size)
        return;

    if(rubi_tx_cursor_high > rubi_tx_cursor_low ||
       (int32_t)RUBI_BUFFER_SIZE - rubi_tx_cursor_low >= size)
    {
        memcpy(&rubi_tx_buffer[rubi_tx_cursor_low], data, size);
        rubi_tx_cursor_low = (rubi_tx_cursor_low + size) % RUBI_BUFFER_SIZE;
    }
    else
    {
        memcpy(
            &rubi_tx_buffer[rubi_tx_cursor_low], data,
            RUBI_BUFFER_SIZE - rubi_tx_cursor_low);
        memcpy(
            rubi_tx_buffer, &data[RUBI_BUFFER_SIZE - rubi_tx_cursor_low],
            size - (RUBI_BUFFER_SIZE - rubi_tx_cursor_low));

        rubi_tx_cursor_low = size - (RUBI_BUFFER_SIZE - rubi_tx_cursor_low);
    }

    rubi_tx_checkfull();
}

// WARNING! this assumes the data will fit
void rubi_tx_enqueue_front(uint8_t* data, int32_t size)
{
    RUBI_ASSERT(0);

    if(!size)
        return;

    if(rubi_tx_cursor_high > rubi_tx_cursor_low || rubi_tx_cursor_high >= size)
    {
        memcpy(&rubi_tx_buffer[rubi_tx_cursor_high - size], data, size);
        rubi_tx_cursor_high -= size;
    }
    else
    {
        memcpy(
            &rubi_tx_buffer[RUBI_BUFFER_SIZE - (size - rubi_tx_cursor_high)],
            data, size - rubi_tx_cursor_high);
        memcpy(
            rubi_tx_buffer, &data[size - rubi_tx_cursor_high],
            rubi_tx_cursor_high);

        rubi_tx_cursor_high = RUBI_BUFFER_SIZE - (size - rubi_tx_cursor_high);
    }

    rubi_tx_checkfull();
}

// NOT counting the header
void rubi_wait_for_tx_and_flock(int32_t datasize)
{
    for(;;)
    {
        while(rubi_tx_avaliable_space() <
              (datasize + (int32_t)sizeof(rubi_dataheader)))
            ;

        rubi_flock();

        if(rubi_tx_avaliable_space() <
           datasize + (int32_t)sizeof(rubi_dataheader))
        {
            rubi_funlock();
        }
        else
        {
            break;
        }
    }
}

inline uint32_t rubi_packed_size(uint32_t size)
{
    return size + size / 7 + 2; // data size, block headers, msg_type, msg_size
}

void rubi_event_continue_tx()
{
    int     i;
    uint8_t data[8];

    rubi_flock();

    for(i = 0;; i++)
    {
        // exit if nothing to send and no transfer in progress
        if(rubi_tx_current_header.msg_type == 0 &&
           rubi_tx_avaliable_space() == RUBI_BUFFER_SIZE)
        {
            break;
        }

        // no transfer in progress but something to send, sice we didn't exit
        if(rubi_tx_current_header.msg_type == 0)
        {
            rubi_tx_current_header =
                *((rubi_dataheader*)rubi_get_tx_chunk(sizeof(rubi_dataheader)));

            block_transfer = rubi_tx_current_header.data_len > 6;
            if(block_transfer)
            {
                rubi_tx_current_header.msg_type |= RUBI_FLAG_BLOCK_TRANSFER;
                blocks_sent = 0;
            }

            if(rubi_tx_cursor_low == -1)
                rubi_tx_cursor_low = rubi_tx_cursor_high;

            rubi_tx_cursor_high =
                (rubi_tx_cursor_high + sizeof(rubi_dataheader)) %
                RUBI_BUFFER_SIZE;
        }

        // now for sure there is something to send
        if(block_transfer && rubi_tx_current_header.data_len != 0)
        {
            uint32_t data_size = (7 > rubi_tx_current_header.data_len)
                                     ? rubi_tx_current_header.data_len
                                     : 7;
            data[0] = RUBI_MSG_BLOCK;
            memcpy((void*)&data[1], rubi_get_tx_chunk(data_size), data_size);

            if(!rubi_can_send_array(
                   rubi_tx_current_header.cob, data_size + 1, data))
            {
                // tx buffer full
                break;
            }

            if(rubi_tx_cursor_low == -1)
                rubi_tx_cursor_low = rubi_tx_cursor_high;

            rubi_tx_cursor_high =
                (rubi_tx_cursor_high + data_size) % RUBI_BUFFER_SIZE;
            rubi_tx_current_header.data_len -= data_size;

            blocks_sent += 1;
        }
        else
        {
            data[0] = rubi_tx_current_header.msg_type;
            data[1] = rubi_tx_current_header.submsg_type;

            if(block_transfer)
            {
                memcpy(data + 2, &blocks_sent, sizeof(blocks_sent));
                if(!rubi_can_send_array(rubi_tx_current_header.cob, 3, data))
                {
                    break;
                }
                blocks_sent = 0;
            }
            else
            {
                memcpy(
                    data + 2,
                    rubi_get_tx_chunk(rubi_tx_current_header.data_len),
                    rubi_tx_current_header.data_len);
                if(!rubi_can_send_array(
                       rubi_tx_current_header.cob,
                       rubi_tx_current_header.data_len + 2,
                       data))
                {
                    break;
                }
            }

            if(rubi_tx_cursor_low == -1 && rubi_tx_current_header.data_len > 0)
            {
                rubi_tx_cursor_low = rubi_tx_cursor_high;
            }

            rubi_tx_cursor_high =
                (rubi_tx_cursor_high + rubi_tx_current_header.data_len) %
                RUBI_BUFFER_SIZE;

            rubi_tx_current_header.msg_type = 0;
        }
    }

    rubi_funlock();
}

void rubi_send_infotext(uint8_t field_id, const char* text, int32_t len)
{
    RUBI_ASSERT(len <= 255);
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, RUBI_MSG_INFO,
                         field_id, len};

    rubi_wait_for_tx_and_flock(len);

    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
    rubi_tx_enqueue_back((uint8_t*)text, len);

    rubi_funlock();
}

void rubi_send_empty(uint8_t msg_type)
{
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, msg_type, 0, 0};

    rubi_wait_for_tx_and_flock(0);

    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));

    rubi_funlock();
}

void rubi_send_infonumber(uint8_t field_id, uint8_t num)
{
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, RUBI_MSG_INFO,
                         field_id, 1};

    rubi_wait_for_tx_and_flock(1);

    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
    rubi_tx_enqueue_back(&num, sizeof(num));

    rubi_funlock();
}

void rubi_send_ffdata(
    uint8_t ff_id, uint8_t ff_type, uint8_t* data, int32_t len)
{
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, ff_type, ff_id,
                         len};

    rubi_wait_for_tx_and_flock(len);

    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
    rubi_tx_enqueue_back(data, len);

    rubi_funlock();
}

void rubi_send_error(
    uint8_t error_type, uint32_t error_code, uint8_t* data, int32_t len)
{
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, RUBI_MSG_EVENT,
                         error_type, len + sizeof(error_code)};

    rubi_wait_for_tx_and_flock(h.data_len);

    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
    rubi_tx_enqueue_back((uint8_t*)&error_code, sizeof(error_code));
    rubi_tx_enqueue_back(data, len);

    rubi_funlock();
    rubi_event_continue_tx();
}

void rubi_send_log(uint8_t level, uint8_t* data, int32_t len)
{
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid, RUBI_MSG_EVENT,
                         level, len};

    rubi_wait_for_tx_and_flock(h.data_len);

    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
    rubi_tx_enqueue_back(data, len);

    rubi_funlock();
    rubi_event_continue_tx();
}

void rubi_send_command(uint8_t command_id, uint8_t* data, uint8_t len)
{
    rubi_dataheader h = {RUBI_ADDRESS_RANGE1_LOW + rubi_nodeid,
                         RUBI_MSG_COMMAND, command_id, len};

    rubi_wait_for_tx_and_flock(0);
    rubi_tx_enqueue_back((uint8_t*)&h, sizeof(h));
    rubi_tx_enqueue_back(data, len);

    rubi_funlock();
    rubi_event_continue_tx();
}
