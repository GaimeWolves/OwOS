set(SOURCES main.c)

add_executable(editor ${SOURCES})

target_compile_options(editor PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(editor PRIVATE -Og -g -Wl,-E)
target_include_directories(editor PRIVATE ${CMAKE_SYSROOT}/usr/include)

target_link_libraries(editor PRIVATE ncurses)

install(TARGETS editor)