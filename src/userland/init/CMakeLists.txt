set(SOURCES main.c)

add_executable(init ${SOURCES})

target_compile_options(init PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(init PRIVATE -Og -g -Wl,-E)
target_include_directories(init PRIVATE ${CMAKE_SYSROOT}/usr/include)

install(TARGETS init)
