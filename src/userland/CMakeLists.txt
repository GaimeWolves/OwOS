include(${CMAKE_SOURCE_DIR}/CMakeToolchain.txt)

add_subdirectory(libc)
add_subdirectory(ld-owos)
add_subdirectory(test)
add_subdirectory(editor)
add_subdirectory(cat)
add_subdirectory(ls)
add_subdirectory(sh)
add_subdirectory(init)
