#!/bin/bash

curr_dir=$(dirname "$0")
echo "curr_dir is ${curr_dir}"
rm -rf /opt/bin/*
echo "==========version8================"
echo "------------start to compile mbr-----------------"
nasm -I ${curr_dir}/include/ -o /opt/bin/mbr.bin ${curr_dir}/mbr.s
echo "----------------write mbr to disk----------"
dd if=/opt/bin/mbr.bin of=/opt/bochs/hd60M.img bs=512 count=1 seek=0 conv=notrunc

echo "-----------compile loader------------------"
nasm -I ${curr_dir}/include/ -o /opt/bin/loader.bin ${curr_dir}/loader.s
echo "------------------write loader to disk----------"
dd if=/opt/bin/loader.bin of=/opt/bochs/hd60M.img bs=512 count=4 seek=2 conv=notrunc



echo "-------compile print.s----"
nasm -f elf -o /opt/bin/print.o ${curr_dir}/lib/kernel/print.s

echo "-------compile main.c-----"
gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -o /opt/bin/main.o kernel/main.c

echo "-------compile init.c-----"
gcc -m32 -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -o /opt/bin/init.o kernel/init.c


echo "------compile interrupt.c---"
gcc -m32  -I lib/kernel/ -I lib/ -I kernel/ -c -fno-builtin -o /opt/bin/interrupt.o kernel/interrupt.c

echo "-----compile kernel.s----"
nasm -f elf -o /opt/bin/kernel.o kernel/kernel.s


echo "-----link main.o and other .o file to generate kernel.bin file----"
ld -m elf_i386  -Ttext 0xc0001500 -e main -o /opt/bin/kernel.bin /opt/bin/main.o /opt/bin/init.o /opt/bin/interrupt.o /opt/bin/print.o /opt/bin/kernel.o


echo "------compile kernel and write to disk-----"
dd if=/opt/bin/kernel.bin of=/opt/bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc


echo "-----start to open computer to run os!------"
cd /opt/bochs;/opt/bochs/bin/bochs -f /opt/bochs/bochsrc.disk
