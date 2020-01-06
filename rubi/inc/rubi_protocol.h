#pragma once

#include "protocol_defs.h"
#include "rubi.h"
#include "rubi_can.h"

typedef struct rubi_dataheader
{
    uint16_t cob;
    uint8_t  msg_type;
    uint8_t  submsg_type;
    uint8_t  data_len;
} rubi_dataheader;

#define RUBI_BUFFER_SIZE (0x1ff + 8 * sizeof(rubi_dataheader))
#define RUBI_ARBITRATION_TIME0 4000
#define RUBI_ARBITRATION_TIME1 1111

void rubi_protocol_init(void);
void rubi_attempt_handshake(void);
void rubi_continue_init();

uint8_t rubi_tryaddress(void);

void rubi_ffdata_inbound(uint8_t* data);
void rubi_command_inbound(uint8_t command, uint8_t parameter);

void rubi_send_empty(uint8_t msg_type);
void rubi_send_infotext(uint8_t field_id, const char* text, int32_t len);
void rubi_send_infonumber(uint8_t field_id, uint8_t num);
void rubi_send_ffdata(
    uint8_t field_id, uint8_t ff_type, uint8_t* data, int32_t len);
void rubi_send_error(
    uint8_t error_type, uint32_t error_code, uint8_t* data, int32_t len);
void rubi_send_log(uint8_t level, uint8_t* data, int32_t len);
void rubi_send_command(uint8_t command_id, uint8_t* data, uint8_t len);
