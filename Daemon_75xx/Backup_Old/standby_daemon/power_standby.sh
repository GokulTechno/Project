#!/bin/bash

if [ "$1" == "1" ]
then

/usr/share/scripts/backlight 0
#/sbin/getty -L /dev/ttyGS0 115200 vt100 &

elif [ "$1" == "2" ]
then

/usr/share/scripts/backlight 1

elif [ "$1" == "3" ]
then

old=`cat /usr/share/status/backlight`
sh /usr/share/scripts/backlight $old

fi
