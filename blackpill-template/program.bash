#!/bin/bash

#TODO: remove sleep 1s!
#made by cezar, send death threats to cezare22@gmail.com

PROJECT_NAME="template"

INTERFACE="-f interface/stlink-v2.cfg" #uncomment for ST-LINK
#INTERFACE="-f interface/jlink.cfg" #uncomment for JLINK

MCU="-f target/stm32f1x.cfg" #uncomment for STM32F1*

# surely you're not debugging two targets at once... right?
killall -9 openocd
if ! make -C $PWD
then
    exit 1
fi

echo "Reset halt & verify... $MCU"
VERIFY_OUTPUT=$(openocd $INTERFACE $MCU \
 -c init -c "reset halt"\
 -c "verify_image $PWD/build/$PROJECT_NAME.elf"\
 -c shutdown 2>&1)

echo $VERIFY_OUTPUT

if echo "$VERIFY_OUTPUT" | grep verified
then
    echo "Target already programmed!"
    nohup openocd $INTERFACE $MCU &>/dev/null &
    #wait for gdb server to init, can be fixed with netcat or sth
    sleep 1
    exit 0
fi

if echo "$VERIFY_OUTPUT" | grep "Error: open failed"
then
    echo "Couldn't connect to the target!"
    exit 1
fi

if ! echo "$VERIFY_OUTPUT" | grep "Error: checksum mismatch" 1> /dev/null
then
    echo "An unknown error has occured!"
    exit 1
fi

echo "Reprogramming target..."

nohup openocd $INTERFACE $MCU\
 -c init -c targets -c "reset halt"\
 -c "flash write_image erase $PWD/build/$PROJECT_NAME.elf"\
 -c "verify_image $PWD/build/$PROJECT_NAME.elf" &>/dev/null &

#same as in 32
sleep 2

exit 0