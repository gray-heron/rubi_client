#pragma once

#include "rubi_auxiliary.h"
#include "rubi_can.h"
#include "rubi_capabilities_enumerator.h"
#include "rubi_internals.h"
#include "rubi_protocol.h"

extern uint32_t rubi_time_sec;
extern uint32_t rubi_time_ms;

void rubi_singleboard_init();
// id must be persistent!
void rubi_multiboard_init(char *id);

void rubi_sleep(void);
void rubi_wakeup(void);

void rubi_callback_operational();
void rubi_callback_lost();
void rubi_callback_error();
void rubi_callback_prepare_sleep();
void rubi_callback_wakeup();
void rubi_callback_shutdown();

void rubi_fatal_error(char *msg);
void rubi_error(char *msg);
void rubi_log(char *msg);
void rubi_info(char *msg);

// === call those from your platform-specific code
void rubi_event_tick_ms(void);
void rubi_event_inbound(rubi_can_msg_t msg);
void rubi_event_continue_tx(void);

#define RUBI_ASSERT(cond)                                                      \
    if (!(cond))                                                               \
        rubi_fire_assert(__LINE__, __FILE__);
