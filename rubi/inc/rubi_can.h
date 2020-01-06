#ifndef CAN_H
#define CAN_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
    uint8_t  data_length;
    int8_t   data[8];
    uint16_t id;
} rubi_can_msg_t;

// allows to pass platform-specific CAN-device identifier for platforms where
// multiple CAN devices are avaliable.
void rubi_select_can(void* can);

void rubi_can_init(void);

void rubi_can_add_filter(
    uint16_t id, uint16_t mask, uint8_t fifo, uint8_t number);

void rubi_flock(void);
void rubi_funlock(void);

bool rubi_can_send_array(int16_t id, uint8_t data_length, uint8_t* data);

#endif
