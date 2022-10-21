set(SOURCES main.c)

add_executable(ld-owos.so ${SOURCES})

target_compile_options(ld-owos.so PRIVATE -Og -g -Wall -Wextra -Werror)
target_link_options(ld-owos.so PRIVATE -nostdlib -Og -g -Wl,-E LINKER:--no-dynamic-linker)
target_link_libraries(ld-owos.so PRIVATE c_static)
target_include_directories(ld-owos.so PRIVATE ${CMAKE_SYSROOT}/usr/include)

install(TARGETS ld-owos.so RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)