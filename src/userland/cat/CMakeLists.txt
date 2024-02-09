set(SOURCES main.c)

add_executable(cat ${SOURCES})

target_compile_options(cat PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(cat PRIVATE -Og -g -Wl,-E)
target_include_directories(cat PRIVATE ${CMAKE_SYSROOT}/usr/include)

install(TARGETS cat)