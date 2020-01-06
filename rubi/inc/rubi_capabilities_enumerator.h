#pragma once
#include "inttypes.h"

void rubi_capabilities_enumeration_start();
bool_t rubi_capabilities_enumeration_continue();

uint8_t rubi_get_field_size(uint32_t id);
uint8_t rubi_get_field_access(uint32_t id);

// computes new crc and returns ptr to relayfield if crc differs,
// or returns nullptr if field didn't chage
uint32_t* rubi_field_update_crc(uint32_t id, uint32_t size);

void rubi_write_ffdata(uint8_t id, uint8_t* ptr);
