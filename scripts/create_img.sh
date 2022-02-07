#!/bin/sh

set -e

die() {
    echo "$*"
    exit 1
}

safe_exit() {
    if [ -d "$BIN_DIR"/mnt ]; then
        if kill -0 "$pid"; then
            guestunmount "$BIN_DIR"/mnt
	fi
    fi
}

trap safe_exit EXIT

if [ ! -e "$BIN_DIR"/img ]; then
    qemu-img create "$BIN_DIR"/img 100M
    mkfs.ext2 -q "$BIN_DIR"/img
fi

mkdir -p "$BIN_DIR"/mnt

guestmount -a "$BIN_DIR"/img -m /dev/sda --pid-file mount.pid "$BIN_DIR"/mnt
pid="$(cat mount.pid)"
rm -rf "${BIN_DIR:?}"/mnt/*
cp -r "$BIN_DIR"/sysroot/* "$BIN_DIR"/mnt
guestunmount "$BIN_DIR"/mnt

timeout=10

count=$timeout
while kill -0 "$pid" 2>/dev/null && [ $count -gt 0 ]; do
    sleep 1
    count=$((count+1))
done

if [ $count -eq 0 ]; then
    echo "$0: wait for guestmount to exit failed after $timeout seconds"
    exit 1
fi

chmod 0666 "$BIN_DIR"/img
