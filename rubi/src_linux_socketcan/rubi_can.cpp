extern "C"
{
#include "rubi.h"
}

#include "socketcan.h"

#include <atomic>
#include <mutex>
#include <random>
#include <thread>

std::mutex rubi_can_lock;

#define FILTER_SLOTS 8

typedef struct
{
    uint16_t id;
    uint16_t mask;
    uint8_t fifo;
} rubi_filter_t;

rubi_filter_t rubi_filters[FILTER_SLOTS];

SocketCan *rubi_socketcan;
boost::optional<std::thread> can_thread;

void rubi_flock(void) {
    rubi_can_lock.lock();
}

void rubi_funlock(void) {
    rubi_can_lock.unlock();
}

bool rubi_msg_matches_filter(rubi_can_msg_t *msg, rubi_filter_t *filter)
{
    return (msg->id & filter->mask) == (filter->id & filter->mask);
};

void CanThread()
{
    rubi_can_msg_t tmp_msg;

    while (true)
    {
        while (auto msg_in = rubi_socketcan->can_receive(0))
        {
            tmp_msg.data_length = std::get<1>(*msg_in).size();
            tmp_msg.id = std::get<0>(*msg_in);

            bool accept = false;
            for(int f = 0; f < FILTER_SLOTS; f++){
                if(!rubi_filters[f].id)
                    continue;

                if((tmp_msg.id & rubi_filters[f].mask) ==
                    (rubi_filters[f].id & rubi_filters[f].mask))
                {
                    accept = true;
                }
            }

            if(!accept)
                continue;

            memcpy(tmp_msg.data, std::get<1>(*msg_in).data(),
                   std::get<1>(*msg_in).size());

            rubi_event_inbound(tmp_msg);
        }

        rubi_delay(10);
    }
}

static char rubi_canname[32];

void rubi_select_can(void* can){
    memcpy(rubi_canname, can, sizeof(rubi_canname));;
}

void rubi_can_init(void)
{
    rubi_socketcan = new SocketCan(rubi_canname);
    can_thread.emplace(CanThread);
}

void rubi_can_add_filter(uint16_t id, uint16_t mask, uint8_t fifo, uint8_t number)
{
    RUBI_ASSERT(number < FILTER_SLOTS);
    rubi_filters[number] = {id, mask, fifo};
}

bool rubi_can_send_array(int16_t id, uint8_t data_length, uint8_t *data)
{
    return rubi_socketcan->can_send(
               std::make_pair(id, std::vector<uint8_t>(data, data + data_length))) == 0;
}