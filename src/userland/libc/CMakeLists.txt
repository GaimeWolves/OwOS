set(SOURCES
    test.cpp
    stdio.cpp
    stdlib.cpp
    unistd.cpp
    string.cpp
    dlfcn.cpp
    math.cpp)

set(HEADERS
    sys/arch/i386/syscall.h
    bits/FILE.h
    test.h
    sys/types.h
    errno.h
    stdlib.h
    stdio.h
    time.h
    unistd.h
    string.h
    stdarg.h
    stdint.h
    dlfcn.h
    math.h
    assert.h
    ctype.h
    locale.h
    setjmp.h
    signal.h)

add_library(crt0 STATIC sys/arch/i386/crt0.S)
add_library(crti STATIC sys/arch/i386/crti.S)
add_library(crtn STATIC sys/arch/i386/crtn.S)

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

add_library(c SHARED ${SOURCES} ${HEADERS})

target_compile_options(c PRIVATE -ffreestanding -fpic -pie -O0 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-omit-frame-pointer)
target_link_options(c PRIVATE -ffreestanding -O0 -nostdlib -pie -Wl,-E,-shared)
target_include_directories(c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(c PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS})

set_target_properties(c PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(c PROPERTIES VERSION 1.0)
set_target_properties(c PROPERTIES SOVERSION 1)

add_dependencies(c crt0 crti crtn)

install(TARGETS c FILE_SET HEADERS)

add_library(c_static STATIC ${SOURCES} ${HEADERS})

set_target_properties(c_static PROPERTIES OUTPUT_NAME c)

target_compile_options(c_static PRIVATE -ffreestanding -fpic -pie -O0 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-omit-frame-pointer)
target_link_options(c_static PRIVATE -ffreestanding -O0 -nostdlib -pie)
target_include_directories(c_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(c_static PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${HEADERS})

set_target_properties(c_static PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(c_static PROPERTIES VERSION 1.0)

install(TARGETS c_static FILE_SET HEADERS)

# These things will all be embedded into libc.so
file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/libpthread.so" "INPUT(libc.so)")
file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/libdl.so" "INPUT(libc.so)")
file(WRITE "${CMAKE_INSTALL_PREFIX}/lib/libm.so" "INPUT(libc.so)")