#!/bin/sh

qemu-system-i386 -s -S \
-no-reboot \
-no-shutdown \
-chardev stdio,id=char0,mux=on,logfile=log.txt,signal=off \
-serial chardev:char0 \
-drive id=disk,file=bin/img,if=none \
-device ahci,id=ahci \
-device ide-hd,drive=disk,bus=ahci.0 \
-smp 4 \
-d int,cpu_reset,guest_errors \
-D debuglog.txt &
sleep 1
