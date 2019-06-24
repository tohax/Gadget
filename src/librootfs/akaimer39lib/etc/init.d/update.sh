#!/bin/sh
VAR1="/mnt/zImage"
VAR2="/mnt/root.sqsh4"
VAR3="/mnt/root.jffs2"

if [ -f $VAR1 ]; then
echo "Updating zImage..."
/usr/bin/updater local K=${VAR1}
fi

if [ -f $VAR2 ]; then
echo "Updating root.sqsh4..."
/usr/bin/updater local MTD1=${VAR2}
fi

if [ -f $VAR3 ]; then
echo "Updating root.jffs2..."
/usr/bin/updater local MTD2=${VAR3}
fi

/sbin/reboot
