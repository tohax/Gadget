#!/bin/sh
telnetd
if pidof camera.sh; then kill `pidof camera.sh`; fi
if pidof record_video; then kill -SIGINT `pidof record_video`; fi
Server=$(cat /etc/network/interfaces | grep server= | cut -d "=" -f 2)
#sleep 3
rdate -s $Server
hwclock --systohc
echo heartbeat > /sys/class/leds/r_led/trigger
sleep 1
echo heartbeat > /sys/class/leds/g_led/trigger
find /mnt/ -name "*index" -exec rm {} \;
time=`date +%Y%m%d`
echo `date` > /etc/`hostname`_on.txt
rsync -avW --size-only --no-perms --numeric-ids --remove-source-files --password-file=/etc/.rsync /etc/`hostname`_on.txt root@$Server::log/$time/
rsync -avW --size-only --no-perms --numeric-ids --remove-source-files --log-file=/etc/`hostname`_ready.txt --password-file=/etc/.rsync /mnt/ root@$Server::counter_1/
rsync -avW --size-only --no-perms --numeric-ids --remove-source-files --password-file=/etc/.rsync /etc/`hostname`_ready.txt root@$Server::log/$time/
find /mnt/[1-9]* -type d -delete 2>/dev/null
echo none > /sys/class/leds/g_led/trigger
echo none > /sys/class/leds/r_led/trigger
echo 0 > /sys/class/leds/g_led/brightness
echo 1 > /sys/class/leds/r_led/brightness
sync && echo 3 > /proc/sys/vm/drop_caches
