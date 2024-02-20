add_subdirectory(sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH})

list(TRANSFORM LIBC_SYSDEPS_SOURCES PREPEND "sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH}/")
list(TRANSFORM LIBC_SYSDEPS_HEADERS PREPEND "sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH}/")

set(SOURCES
    ${LIBC_SYSDEPS_SOURCES}
    __debug.c
    __heap.c
    __heap.h
    __malloc.c
    __printf.c
    __printf.h
    assert.c
    ctype.c
    dirent.c
    dlfcn.c
    errno.c
    fcntl.c
    locale.c
    math.c
    signal.c
    stdio.c
    stdlib.c
    string.c
    sys/internals.c
    sys/ioctl.c
    sys/mman.c
    sys/stat.c
    sys/wait.c
    termios.c
    time.c
    unistd.c)

set(HEADERS
    __debug.h
    assert.h
    bits/attributes.h
    bits/FILE.h
    bits/environ.h
    bits/guards.h
    ctype.h
    dirent.h
    dlfcn.h
    errno.h
    fcntl.h
    limits.h
    locale.h
    math.h
    setjmp.h
    signal.h
    stdarg.h
    stdbool.h
    stddef.h
    stdint.h
    stdio.h
    stdlib.h
    string.h
    sys/auxv.h
    sys/internals.h
    sys/ioctl.h
    sys/mman.h
    sys/stat.h
    sys/syscall.h
    sys/types.h
    sys/wait.h
    termios.h
    time.h
    unistd.h)

add_library(crt0 STATIC sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH}/crt0.S)
add_library(crti STATIC sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH}/crti.S)
add_library(crtn STATIC sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH}/crtn.S)

add_custom_command(
    TARGET crt0
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${CMAKE_INSTALL_PREFIX}/lib/crt0.o
)

add_custom_command(
    TARGET crti
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crti> ${CMAKE_INSTALL_PREFIX}/lib/crti.o
)

add_custom_command(
    TARGET crtn
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crtn> ${CMAKE_INSTALL_PREFIX}/lib/crtn.o
)

add_library(c_objlib OBJECT ${SOURCES} ${HEADERS} ${LIBC_SYSDEPS_HEADERS})
target_compile_options(c_objlib PRIVATE -ffreestanding -nostdlib -fpic -pie -Og -g -Wall -Wextra -Werror -fno-exceptions -fno-omit-frame-pointer)
target_include_directories(c_objlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH})
target_compile_definitions(c_objlib PRIVATE __LIBC_ARCH=${LIBC_ARCH} __LIBC_SYSTEM=${LIBC_SYSTEM})
# target_compile_definitions(c_objlib PRIVATE __LIBC_DEBUG)

add_library(c SHARED $<TARGET_OBJECTS:c_objlib>)
target_link_libraries(c PRIVATE gcc)
target_link_options(c PRIVATE -ffreestanding -nostdlib)

target_sources(c PUBLIC FILE_SET base_headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS})

target_sources(c PUBLIC FILE_SET sysdeps_headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/sysdeps/${LIBC_SYSTEM}/${LIBC_ARCH}
    FILES ${LIBC_SYSDEPS_HEADERS})

set_target_properties(c PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(c PROPERTIES VERSION 1)
set_target_properties(c PROPERTIES SOVERSION 1)

add_dependencies(c crt0 crti crtn)

install(TARGETS c FILE_SET base_headers FILE_SET sysdeps_headers RUNTIME LIBRARY)

add_library(c_static STATIC $<TARGET_OBJECTS:c_objlib>)
set_target_properties(c_static PROPERTIES OUTPUT_NAME c)
target_link_libraries(c_static PRIVATE gcc)
target_link_options(c_static PRIVATE -ffreestanding -nostdlib)

set_target_properties(c_static PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(c_static PROPERTIES VERSION 1.0)

install(TARGETS c_static RUNTIME LIBRARY)

# These things will all be embedded into libc.so
file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/libpthread.so" "INPUT(libc.so)")
file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/libdl.so" "INPUT(libc.so)")
file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/libm.so" "INPUT(libc.so)")
