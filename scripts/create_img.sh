#!/bin/sh

set -e

die() {
    echo "$*"
    exit 1
}

is_mounted=0
has_loopdevice=0

safe_exit() {
    if [ $is_mounted -eq 1 ]; then
        sudo umount "$BIN_DIR"/mnt
    fi
    if [ $has_loopdevice -eq 1 ]; then
        sudo losetup -D "$dev"
    fi
    if [ -d "$BIN_DIR"/mnt ]; then
        if kill -0 "$pid" 2>/dev/null; then
            guestunmount "$BIN_DIR"/mnt
        fi
    fi
}

trap safe_exit EXIT

install_grub=0

if [ ! -e "$BIN_DIR"/img ]; then
    printf "\nCreating initial image\n"

    install_grub=1
    qemu-img create -f raw "$BIN_DIR"/img 100M
    parted -s "$BIN_DIR"/img mklabel gpt mkpart boot ext3 1MiB 8MiB mkpart primary ext2 8MiB 100% -a minimal set 1 bios_grub on

    printf "\nAllocating loop device\n"

    dev=$(sudo losetup --find --partscan --show "$BIN_DIR"/img)
    has_loopdevice=1

    sudo mke2fs -b 4096 "${dev}"p2
fi

mkdir -p "$BIN_DIR"/mnt

printf "\nMounting image\n"

guestmount -a "$BIN_DIR"/img -m /dev/sda2 --pid-file mount.pid "$BIN_DIR"/mnt
pid="$(cat mount.pid)"
rm mount.pid
rm -rf "${BIN_DIR:?}"/mnt/*
printf "\nCopying Files\n"
cp -r "$BIN_DIR"/sysroot/* "$BIN_DIR"/mnt
cp -r "$BIN_DIR"/../sysroot/* "$BIN_DIR"/mnt
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

# Using sudo for now as this now requires `losetup` to install GRUB 2 on the image

if [ $install_grub -eq 1 ]; then
    printf "\nInstalling grub\n"

    sudo mount "${dev}"p2 "$BIN_DIR"/mnt
    is_mounted=1

    sudo grub-install --target="i386-pc" --boot-directory="$BIN_DIR"/mnt/boot --no-floppy --modules="normal part_gpt ext2 fat multiboot" "${dev}"

    sudo umount "$BIN_DIR"/mnt
    is_mounted=0

    sudo losetup -D "${dev}"
    has_loopdevice=0
fi

printf "\nFinished image creation\n"

chmod 0666 "$BIN_DIR"/img
