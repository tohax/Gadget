#!/bin/sh
if pidof on.sh; then kill `pidof on.sh`; fi
if pidof rsync; then kill `pidof rsync`; fi
echo 0 > /sys/class/leds/r_led/brightness
echo 0 > /sys/class/leds/g_led/brightness
if [ ! `pidof record_video` ]; then
 /etc/init.d/camera.sh &
fi
