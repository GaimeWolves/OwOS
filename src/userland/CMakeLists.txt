include(${CMAKE_SOURCE_DIR}/CMakeToolchain.txt)

add_subdirectory(libc)
add_subdirectory(ld-owos)
add_subdirectory(test)
add_subdirectory(sh)
add_subdirectory(init)