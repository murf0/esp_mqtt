#!/bin/sh
make clean
make C99=YES
rm build/thebutton*
make
echo "Flashing"
make flash
screen /dev/tty.wchusbserial14* 115200

#/Volumes/esp-open-sdk/esptool/esptool.py -p /dev/tty.wchusbserial14* write_flash 0x00000 firmware/0x00000.bin 0x3c000 firmware/0x3c000.bin 0x12000 webpages.espfs && screen /dev/tty.wchusbserial14* 115200
