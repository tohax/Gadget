#!/bin/sh
killall -9 telnetd on.sh rsync
echo 0 > /sys/class/leds/r_led/brightness
echo 0 > /sys/class/leds/g_led/brightness
if [ ! `pidof record_video` ]; then
 /etc/init.d/camera.sh &
fi
