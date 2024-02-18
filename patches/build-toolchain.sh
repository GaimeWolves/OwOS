#!/bin/sh

set -e

if ! [ -d "$ROOTDIR" ]; then
  echo "Set ROOTDIR variable to the project root"
  exit
fi

# TODO: handle trailing slashes correctly
if [ "$PWD" != "$ROOTDIR/patches" ]; then
  echo "Run this script in the 'ROOTDIR/patches' directory"
  exit
fi

BINUTILS_VERSION=2.42
GCC_VERSION=13.2.0

TARGET=i686-owos
PREFIX="$ROOTDIR/toolchain-hosted"
SYSROOT="$ROOTDIR/bin/sysroot"

THREADS=4

export TARGET
export PREFIX
export SYSROOT
export PATH="$PATH:$PREFIX/bin"

# reset environment as the configure script will check for these variables to determine the host gcc installation
unset CC
unset CXX
unset LD
unset AR
unset AS
unset OBJCOPY
unset OBJDUMP

# clean build files
rm -f gcc-$GCC_VERSION.tar.gz
rm -f binutils-$BINUTILS_VERSION.tar.gz
rm -rf build-binutils
rm -rf build-gcc
rm -rf binutils-$BINUTILS_VERSION
rm -rf gcc-$GCC_VERSION

# get binutils and gcc
curl -O https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
tar -xzvf binutils-$BINUTILS_VERSION.tar.gz

curl -O https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
tar -xzvf gcc-$GCC_VERSION.tar.gz

# make and install binutils
mkdir -p build-binutils
cd binutils-$BINUTILS_VERSION && for file in ../binutils/*.patch; do patch -p1 <"$file"; done && cd ..
cd build-binutils && ../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-werror --enable-shared --disable-nls && cd ..
cd build-binutils && make -j $THREADS && cd ..
cd build-binutils && make -j $THREADS install && cd ..

# make and install gcc
mkdir -p build-gcc
cd gcc-$GCC_VERSION && for file in ../gcc/*.patch; do patch -p1 <"$file"; done && cd ..
cd build-gcc && ../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$SYSROOT" \
  --enable-languages=c,c++ \
  --enable-default-pie \
  --enable-lto \
  --enable-initfini-array \
  --enable-shared \
  --disable-nls \
  --disable-hosted-libstdcxx \
  --disable-libstdcxx-pch \
  --disable-libstdcxx-visibility \
  --disable-libstdcxx-dual-abi \
  --disable-libstdcxx-threads \
  --disable-libstdcxx-filesystem-ts \
  --disable-libstdcxx-backtrace \
  --disable-tls \
  --disable-wchar_t \
  --disable-multilib \
  --disable-largefile && cd ..
cd build-gcc && make -j $THREADS all-gcc all-target-libgcc && cd ..
cd build-gcc && make -j $THREADS install-gcc install-target-libgcc && cd ..

# make and install libstdc++-v3
cd build-gcc && make -j $THREADS all-target-libstdc++-v3 && make -j $THREADS install-target-libstdc++-v3 && cd ..

# clean build files
rm -f gcc-$GCC_VERSION.tar.gz
rm -f binutils-$BINUTILS_VERSION.tar.gz
rm -rf build-binutils
rm -rf build-gcc
rm -rf binutils-$BINUTILS_VERSION
rm -rf gcc-$GCC_VERSION