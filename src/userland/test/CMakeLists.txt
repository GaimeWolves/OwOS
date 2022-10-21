set(SOURCES main.c)

add_executable(test ${SOURCES})

target_compile_options(test PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(test PRIVATE -Og -g -Wl,-E)
target_include_directories(test PRIVATE ${CMAKE_SYSROOT}/usr/include)

install(TARGETS test)