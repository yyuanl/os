#!/bin/bash

make clean

sleep 1

make all

echo "-------------------------------------------------------------------------------make end----------------------------------------------------------------------------"

sleep 1

make image

echo "-------------------------------------------------------------------------------finish to install os,prepare to start os----------------------------------------------"

sleep 1

cd /opt/bochs;/opt/bochs/bin/bochs -f /opt/bochs/bochsrc.disk
