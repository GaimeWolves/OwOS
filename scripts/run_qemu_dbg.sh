#!/bin/sh

qemu-system-i386 -s -S \
-no-reboot \
-no-shutdown \
-kernel bin/sysroot/boot/kernel \
-chardev stdio,id=char0,mux=on,logfile=log.txt,signal=off \
-serial chardev:char0 -mon chardev=char0 \
-hda bin/img