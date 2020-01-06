
#include <stdio.h>

#include "rubi.h"

//============================================================

RUBI_UNIVERSAL_BOARD("TestBoard", "1.0", "rubi_generic",
                     "Example project for blackpill.", 10);

RUBI_REGISTER_FIELD(stdout_test, RUBI_READONLY, int32_t)
RUBI_REGISTER_FIELD(loopback_in, RUBI_READONLY, int32_t)
RUBI_REGISTER_FIELD(loopback_out, RUBI_WRITEONLY, int32_t)

RUBI_END()

//============================================================

void rubi_callback_operational() {
    printf("==OPERATIONAL==\n");
}

void rubi_callback_lost() {
    printf("==LOST==\n");
}

void rubi_callback_error() {}

void rubi_callback_prepare_sleep() {}

void rubi_callback_wakeup() {}

void rubi_callback_shutdown() {}


//============================================================

int main(int argc, char** argv)
{
    if(argc != 3){
        printf("Usage: ./board <socketcan_bus_name> <board_id>\n");
        return 1;
    }

    rubi_select_can(argv[1]);
    rubi_multiboard_init(argv[2]);
    rubi_wakeup();

    while (true)
    {
    	printf("%d\n", stdout_test);
        loopback_out = loopback_in;

        rubi_delay(1000);
    }

    return 0;
}
