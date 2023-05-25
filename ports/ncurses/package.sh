#!/bin/sh

wget https://ftp.gnu.org/gnu/ncurses/ncurses-6.4.tar.gz

tar -xzvf ncurses-6.4.tart.gz

mkdir -p build
mkdir -p sysroot

export ROOTDIR="<project-root>"
export OWOS_SYSROOT="$ROOTDIR/bin/sysroot"

export PATH=$PATH:$ROOTDIR/ports:$ROOTDIR/toolchain-hosted/bin
export PKG_CONFIG=i686-owos-pkg-config
export PKG_CONFIG_FOR_BUILD=pkg-config

export CC="i686-owos-gcc"
export CXX="i686-owos-g++"
export AR="i686-owos-ar"
export RANLIB="i686-owos-ranlib"
export READELF="i686-owos-readelf"
export OBJCOPY="i686-owos-objcopy"
export STRIP="i686-owos-strip"
export CXXFILT="i686-owos-c++filt"

cd build
../ncurses-6.4/configure --host=i686-owos --prefix="/usr" --without-cxx-binding --without-cxx --without-develop --with-shared --with-pkg-config --enable-assertions --with-debug --without-tests --without-ada --enable-term-driver --with-build-cc="/bin/gcc"
make
make DESTDIR=$OWOS_SYSROOT install

