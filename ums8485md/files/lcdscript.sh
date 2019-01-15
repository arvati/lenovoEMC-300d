#!/bin/bash
while true
do
convert -size 128x64 -monochrome -pointsize 8 -font slkscr.ttf caption:"$(hostname -I | awk '{print "IP: "$1}') \r Hostname: $HOSTNAME \r Load: $(cat /proc/loadavg | awk '{print $1" "$2" "$3}') \r MemFree: $(free -h | grep Mem | awk '{print $4}')" /dev/shm/out.bmp
/root/lenovoEMC-300d/lcdimg /dev/shm/out.bmp > /dev/null 2>&1
sleep 10
done
