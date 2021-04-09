#!/bin/sh

set -e

die() {
    echo "$*"
    exit 1
}

if [ $(/usr/bin/id -u) != "0" ]; then
    exec sudo -E -- "$0" "$@" || die "this script needs root priviledges"
fi

safe_exit() {
    if [ -d "$BIN_DIR/mnt" ]; then
        rm -rf mnt || ( umount mnt && rm -rf mnt )
    fi
}

trap safe_exit EXIT

qemu-img create "$BIN_DIR/img" 100M
mkfs.ext2 -q "$BIN_DIR/img"
mkdir -p "$BIN_DIR/mnt"
mount "$BIN_DIR/img" "$BIN_DIR/mnt"
cp -r "$BIN_DIR"/sysroot/* "$BIN_DIR/mnt"
umount "$BIN_DIR/mnt"
rm -rf "$BIN_DIR/mnt"
chmod 0666 "$BIN_DIR/img"