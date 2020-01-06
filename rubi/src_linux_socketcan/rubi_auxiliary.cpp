
extern "C"
{
#include "rubi.h"
extern char __rubi_board_id[RUBI_SHORTSTRING_MAX_LEN];
}

#include <boost/crc.hpp>
#include <boost/optional.hpp>


#include <random>
#include <thread>
#include <cstring>

extern char *rubi_linux_id;

boost::optional<std::thread> systick_thread;

void SystickThread()
{
    while (true)
    {
        rubi_event_tick_ms();
        rubi_delay(1);
    }
}

void rubi_auxiliary_init() { systick_thread.emplace(SystickThread); }

uint32_t rubi_crc_nopoly(uint32_t *data, uint32_t datalen)
{
    boost::crc_32_type result;
    result.process_bytes((void *)data, datalen * 4);
    return result.checksum();
}

void rubi_delay(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void rubi_die() { exit(1); }

void rubi_reboot() { exit(0); }

uint32_t rubi_rng()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(
        0, std::numeric_limits<uint32_t>::max());

    return dist(rng);
}
