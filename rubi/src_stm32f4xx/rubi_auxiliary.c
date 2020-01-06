#include "rubi_auxiliary.h"
#include "rubi.h"

#include "stm32f4xx_hal.h"
#include <inttypes.h>

extern CRC_HandleTypeDef hcrc;

typedef struct int96_t
{
    int32_t b_31_0;
    int32_t b_63_32;
    int32_t b_95_64;
} int96_t;

static char rubi_board_id[sizeof(int96_t) * 2 + 1];

static const int96_t *const UID = (void *)0x1FFFF7E8;

static inline char hex_to_char(int8_t hex)
{
    hex = (hex & 0xF) + '0';
    return (hex > '9' && hex < 'A') ? hex + ('A' - '9' - 1) : hex;
}

static void mem_to_str(const void *in, size_t size, char *out)
{
    int8_t *mem = (int8_t *)in;

    uint32_t i;
    for (i = 0; i < size; i++)
    {
        const char c_1 = hex_to_char(mem[size - i - 1] >> 4);
        const char c_2 = hex_to_char(mem[size - i - 1]);

        out[2 * i] = c_1;
        out[2 * i + 1] = c_2;
    }
    out[2 * i] = '\0';
}

char *rubi_get_uid() { return rubi_board_id; }

void rubi_auxiliary_init()
{
    mem_to_str(UID, sizeof(int96_t), rubi_board_id);
    rubi_board_id[sizeof(int96_t) * 2] = '\0';
}

uint32_t rubi_crc_nopoly(uint32_t *data, uint32_t datalen)
{
    return HAL_CRC_Calculate(&hcrc, data, datalen);
}

void rubi_delay(uint32_t ms) { HAL_Delay(ms); }

void rubi_die()
{
    __disable_irq();
    rubi_callback_error();
    while (1)
        ;
}

void rubi_reboot()
{
    rubi_callback_shutdown();
    HAL_NVIC_SystemReset();
}

uint32_t rubi_rng()
{
    // fixme
    return HAL_CRC_Calculate(&hcrc, (uint32_t *)UID_BASE, 3 * sizeof(uint32_t));
}
