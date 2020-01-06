#include "rubi.h"
#include "rubi_auxiliary.h"
#include "rubi_can.h"
#include "rubi_capabilities_enumerator.h"

extern const uint32_t __rubi_decl_count;
extern int8_t *__rubi_typetable;
extern const uint16_t __rubi_update_hz;

uint8_t rubi_state = RUBI_STATE_IDLE;

uint32_t rubi_time_sec;
uint32_t rubi_time_ms;
uint32_t rubi_time_total;

uint32_t rubi_last_keepalive_time = 0;
uint8_t rubi_wake = 0;

char *__rubi_board_id;

static void rubi_keepalive_check();

enum init_type
{
    INIT_TYPE_SINGLEBOARD,
    INIT_TYPE_MULTIBOARD
};

static void rubi_init(char *id)
{
    __rubi_board_id = id;

    rubi_can_init();
    rubi_auxiliary_init();
    rubi_protocol_init();
}

void rubi_singleboard_init() { rubi_init(NULL); }

void rubi_multiboard_init(char *id) { rubi_init(id); }

void rubi_update_fields()
{
    uint32_t i;

    for (i = 0; i < __rubi_decl_count; i++)
    {
        int access = rubi_get_field_access(i);
        if ((access == RUBI_WRITEONLY || access == RUBI_READWRITE))
        {
            uint32_t fsize = rubi_get_field_size(i);
            uint32_t *pptr = rubi_field_update_crc(i, fsize);

            if (pptr != NULL)
            {
                rubi_send_ffdata(i, RUBI_MSG_FIELD, (uint8_t *)pptr, fsize);
            }
        }
    }
}

void rubi_event_tick_ms()
{
    if (rubi_time_total++ == (1000 / __rubi_update_hz))
    {
        if (rubi_state == RUBI_STATE_OPERATIONAL && rubi_wake)
        {
            rubi_update_fields();
            rubi_event_continue_tx();
        }

        rubi_time_total = 0;
    }

    if (rubi_state == RUBI_STATE_OPERATIONAL)
        rubi_keepalive_check();
    else if (rubi_state == RUBI_STATE_INITING)
        rubi_continue_init();

    rubi_time_ms += 1;
    if (rubi_time_ms == 1000)
    {
        rubi_time_sec += 1;
        rubi_time_ms = 0;
    }
}

void rubi_sleep(void)
{
    if (rubi_wake)
        rubi_callback_prepare_sleep();

    rubi_wake = 0;
}

void rubi_wakeup(void)
{
    if (rubi_wake)
        rubi_callback_prepare_sleep();

    rubi_wake = 1;
}

void rubi_keepalive_received() { rubi_last_keepalive_time = rubi_time_sec; }

void rubi_keepalive_check()
{
    if (rubi_time_sec - rubi_last_keepalive_time > 4)
    {
        rubi_callback_lost();
        rubi_attempt_handshake();
    }
}

void rubi_ffdata_inbound(uint8_t *data)
{
    uint8_t id = *data;
    data += 1;
    rubi_write_ffdata(id, data);
}

void rubi_fire_assert(int32_t line, const char *file)
{
    if (rubi_state == RUBI_STATE_OPERATIONAL)
        rubi_send_error(RUBI_ERROR_ASSERT, line, (uint8_t *)file, strlen(file));
    rubi_callback_error();
    rubi_state = RUBI_STATE_ERROR;
    rubi_die();
}

void rubi_fatal_error(char *msg)
{
    uint32_t msg_len = strlen(msg);

    if (rubi_state == RUBI_STATE_OPERATIONAL)
        rubi_send_error(RUBI_ERROR_USER, 0, (uint8_t *)msg, msg_len);

    rubi_die();
}

void rubi_error(char *msg)
{
    uint32_t msg_len = strlen(msg);

    if (rubi_state == RUBI_STATE_OPERATIONAL)
        rubi_send_log(RUBI_EVENT_ERROR, (uint8_t *)msg, msg_len);
}

void rubi_log(char *msg)
{
    uint32_t msg_len = strlen(msg);

    if (rubi_state == RUBI_STATE_OPERATIONAL)
        rubi_send_log(RUBI_EVENT_WARNING, (uint8_t *)msg, msg_len);
}

void rubi_info(char *msg)
{
    uint32_t msg_len = strlen(msg);

    if (rubi_state == RUBI_STATE_OPERATIONAL)
        rubi_send_log(RUBI_EVENT_INFO, (uint8_t *)msg, msg_len);
}
