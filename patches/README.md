## Info

Binutils version used: 2.39
GCC version used: 12.2.0

Patched code is listed in this directory.

This should be automated later.

Execution:
../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-werror --enable-shared --disable-nls
make
make install

../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$SYSROOT" --disable-werror --enable-shared --enable-languages=c,c++ --enable-default-pie --enable-lto --enable-initfini-array
make all-gcc all-target-libgcc
make install-gcc install-target-libgcc