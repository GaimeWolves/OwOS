set(SOURCES main.c)

add_executable(sh ${SOURCES})

target_compile_options(sh PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(sh PRIVATE -Og -g -Wl,-E)
target_include_directories(sh PRIVATE ${CMAKE_SYSROOT}/usr/include)

install(TARGETS sh)