set(SOURCES main.c)

add_executable(ls ${SOURCES})

target_compile_options(ls PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(ls PRIVATE -Og -g -Wl,-E)
target_include_directories(ls PRIVATE ${CMAKE_SYSROOT}/usr/include)

install(TARGETS ls)