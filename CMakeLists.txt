cmake_minimum_required(VERSION 3.18)
project(OwOS C CXX ASM)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(ARCH "i686")

option(DEBUG_KERNEL "Enable kernel debug messages" ON)

set(TOOLCHAIN_PATH ${CMAKE_SOURCE_DIR}/toolchain/bin/${ARCH}-elf-)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}gcc)
set(CMAKE_LINKER ${TOOLCHAIN_PATH}ld)
set(CMAKE_RANLIB ${TOOLCHAIN_PATH}ranlib)
set(CMAKE_STRIP ${TOOLCHAIN_PATH}strip)
set(CMAKE_AR ${TOOLCHAIN_PATH}ar)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/build)

add_subdirectory(src)

add_custom_target(run
    COMMAND qemu-system-i386 -kernel ${CMAKE_INSTALL_PREFIX}/sysroot/boot/kernel
    DEPENDS kernel
    USES_TERMINAL
)

add_custom_target(debug
    COMMAND qemu-system-i386 -s -S -kernel ${CMAKE_INSTALL_PREFIX}/sysroot/boot/kernel & cd ${CMAKE_SOURCE_DIR} && gdb -ex "target remote localhost:1234" -ex "symbol-file ${CMAKE_INSTALL_PREFIX}/sysroot/boot/kernel"
    DEPENDS kernel
    USES_TERMINAL
)