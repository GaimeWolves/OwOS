cmake_minimum_required(VERSION 3.13)
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
set(GDB ${TOOLCHAIN_PATH}gdb)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/build)

add_subdirectory(src)

add_custom_command(
	OUTPUT ${CMAKE_INSTALL_PREFIX}/img
	COMMAND ${CMAKE_COMMAND} -E env "BIN_DIR=${CMAKE_INSTALL_PREFIX}" ${CMAKE_SOURCE_DIR}/scripts/create_img.sh
	DEPENDS kernel
	USES_TERMINAL
)

add_custom_target(image
	COMMAND echo "Making image"
	DEPENDS ${CMAKE_INSTALL_PREFIX}/img
	VERBATIM
)

add_custom_target(run
	COMMAND qemu-system-i386 -no-reboot -no-shutdown -kernel ${CMAKE_INSTALL_PREFIX}/sysroot/boot/kernel -chardev stdio,id=char0,mux=on,logfile=${CMAKE_SOURCE_DIR}/log.txt,signal=off -serial chardev:char0 -mon chardev=char0 -hda ${CMAKE_INSTALL_PREFIX}/img -smp 4
	DEPENDS ${CMAKE_INSTALL_PREFIX}/img
	USES_TERMINAL
)

add_custom_target(debug
	COMMAND qemu-system-i386 -s -S -no-reboot -no-shutdown -kernel ${CMAKE_INSTALL_PREFIX}/sysroot/boot/kernel -chardev stdio,id=char0,mux=on,logfile=${CMAKE_SOURCE_DIR}/log.txt,signal=off -serial chardev:char0 -mon chardev=char0 -hda ${CMAKE_INSTALL_PREFIX}/img -smp 4 & cd ${CMAKE_SOURCE_DIR} && sleep 1 && ${GDB} -ex "target remote localhost:1234" -ex "symbol-file ${CMAKE_INSTALL_PREFIX}/sysroot/boot/kernel"
	DEPENDS ${CMAKE_INSTALL_PREFIX}/img
	USES_TERMINAL
)