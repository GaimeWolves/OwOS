#!/bin/sh

set -e

NCURSES_VERSION=6.4

if ! [ -d "$ROOTDIR" ]; then
  echo "Set ROOTDIR variable to the project root"
  exit
fi

# TODO: handle trailing slashes correctly
if [ "$PWD" != "$ROOTDIR/ports/ncurses" ]; then
  echo "Run this script in the 'ROOTDIR/ports/ncurses' directory"
  exit
fi

rm -rf "ncurses-$NCURSES_VERSION.tar.gz"
rm -rf "ncurses-$NCURSES_VERSION"
rm -rf build

wget "https://ftp.gnu.org/gnu/ncurses/ncurses-$NCURSES_VERSION.tar.gz"

tar -xzvf "ncurses-$NCURSES_VERSION.tar.gz"

mkdir -p build

export OWOS_SYSROOT="$ROOTDIR/bin/sysroot"

export PATH="$PATH:$ROOTDIR/ports:$ROOTDIR/toolchain-hosted/bin"
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

cd "./ncurses-$NCURSES_VERSION" && for file in ../patches/*.patch; do patch -p1 <"$file"; done && cd ..
cd build || exit
"../ncurses-$NCURSES_VERSION/configure" \
  --host=i686-owos --prefix="/usr" \
  --without-cxx-binding \
  --without-cxx \
  --without-develop \
  --with-shared \
  --with-pkg-config \
  --enable-assertions \
  --with-debug \
  --without-tests \
  --without-ada \
  --enable-term-driver \
  --with-build-cc="/bin/gcc"
make
make DESTDIR="$OWOS_SYSROOT" install

