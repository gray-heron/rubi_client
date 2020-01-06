
# Rover Universal Board Infrastructure -- client side

## General description

This is a subproject of RUBI that implements the client side for the STM32F0,F1,F4 microcontrollers,
and for Linux using the socketcan.
General description of the system can be found in the [server repository](https://github.com/acriaer/rubi_server).

## Usage example

Here is how *main.c* may look like, when using RUBI to implement a simple weather station:

```C
#include "rubi.h"
[...]

RUBI_UNIVERSAL_BOARD(<client_name>, <firmware_version>, <ROS driver>,
                     <client_description>, <update_frequency>);

RUBI_REGISTER_FIELD(leds, RUBI_READONLY, bool_t, red, green, blue)
RUBI_REGISTER_FIELD(temperature, RUBI_WRITEONLY, int32_t)
RUBI_REGISTER_FIELD(wind, RUBI_WRITEONLY, float, direction, speed)
<other capabilities of the client>

RUBI_END()

[...]

void rubi_callback_operational() {
    SetGPIO(PIN_LED_ACTIVE, GPIO_HIGH);
    open_cover();
}

void rubi_callback_lost() {
    SetGPIO(PIN_LED_ACTIVE, GPIO_LOW);
    close_cover();
}

void rubi_callback_error() {
    motor_emergency_stop();
    close_cover();
}

[...] //other callbacks

void main() {
    [...] //initialize clock, CAN bus, etc

    rubi_init_singleboard(); 

    [...]

    while(1) {
        SetGPIO(PIN_LED_R, leds.red ? GPIO_LOW : GPIO_HIGH);
        SetGPIO(PIN_LED_G, leds.green ? GPIO_LOW : GPIO_HIGH);
        SetGPIO(PIN_LED_B, leds.blue ? GPIO_LOW : GPIO_HIGH);

        temperature = temperature_sensor.GetTemperature();
        wind.direction = wind_sensor.GetWindHeading();
        wind.speed = wind_sensor.GetWindSpeed();
        [...]
    }
}
```

## File structure

- `blackpill-template` -- an example / testing application for STM32F103C8T6 microcontroller on a [blackpill board](https://stm32-base.org/boards/STM32F103C8T6-Black-Pill.html)
- `linux-socketcan-template` -- an example / testing application for Linux that communicates trough socketcan
- `rubi` -- implementation of the rubi client 
  - `src` -- implementation of the core client features (like communication protocol)
  - `src_<platfotm>` -- platform-specific code (like CAN driver)

## Buildting & running the Linux example
This assumes the ROS is configured and rubi-server installed.
```bash
# building
cd linux-socketcan-template
mkdir build
cd build
cmake ..
make

#running
sudo modprobe vcan && sudo ip link add dev vcan1 type vcan && sudo ip link set up vcan1
rosrun rubi_server rubi_server _cans:=vcan1&
./board vcan1 test_board1
```