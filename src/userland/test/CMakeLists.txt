set(SOURCES main.c)

add_executable(test ${SOURCES})

target_compile_options(test PRIVATE -fpic -pie -O0 -Wall -Wextra -Werror)
target_link_options(test PRIVATE -O0 -shared -pie -Wl,-E,-pie)

set_target_properties(test PROPERTIES POSITION_INDEPENDENT_CODE 1)

install(TARGETS test)