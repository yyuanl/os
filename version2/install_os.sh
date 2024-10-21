#!/bin/bash

curr_dir=$(dirname "$0")
echo "curr_dir is ${curr_dir}"
rm -rf /opt/bin/*
echo "==========version2================"
echo "------------start to compile mbr-----------------"
nasm -I ${curr_dir}/include/ -o /opt/bin/mbr.bin ${curr_dir}/mbr.s
echo "----------------write mbr to disk----------"

dd if=/opt/bin/mbr.bin of=/opt/bochs/hd60M.img bs=512 count=1 conv=notrunc

echo "-----------compile loader------------------"
nasm -I ${curr_dir}/include/ -o /opt/bin/loader.bin ${curr_dir}/loader.s
echo "------------------write loader to disk----------"
dd if=/opt/bin/loader.bin of=/opt/bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc

echo "-----start to open computer to run os!------"
cd /opt/bochs;/opt/bochs/bin/bochs -f /opt/bochs/bochsrc.disk
