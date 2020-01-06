#pragma once

#include "rubi_autodefs.h"

#include <inttypes.h>
#include <stddef.h>

void rubi_auxiliary_init();
char *rubi_get_uid();
uint32_t rubi_crc_nopoly(uint32_t *data, uint32_t datalen);
void rubi_delay(uint32_t ms);
void rubi_die();
void rubi_reboot();
uint32_t rubi_rng();
