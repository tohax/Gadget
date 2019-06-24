#!/bin/bash
echo heartbeat > /sys/class/leds/g_led/trigger
sed -i 's/\r$//g' /mnt/interfaces
cp /mnt/interfaces /etc/network/interfaces

####Name
echo `cat /etc/network/interfaces | grep host= | cut -d "=" -f 2` > /etc/sysconfig/HOSTNAME
hostname -F /etc/sysconfig/HOSTNAME

####Net
Server=$(cat /etc/network/interfaces | grep server= | cut -d "=" -f 2)
ifup usb0

while true
do
state=$(cat /sys/devices/platform/ak-hsudc/gadget/net/usb0/operstate)
        if [[ $state = "up" ]]
	then break;
	fi
echo "Connect cable"
sleep 5
done

rdate -s $Server
hwclock --systohc
echo `date`
if [ ! -f /etc/mtab ]; then ln -s /proc/mounts /etc/mtab; fi
umount -l /mnt
yes | /usr/bin/mke2fs -t ext3 /dev/mmcblk0p1
mount /dev/mmcblk0p1 /mnt
rm -rf /mnt/*
echo "Setup finished" >> /mnt/`hostname`_finish
reboot
