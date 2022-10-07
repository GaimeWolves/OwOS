set(SOURCES main.cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(test SHARED ${SOURCES})

target_compile_options(test PUBLIC -ffreestanding -pie -O0 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fno-omit-frame-pointer)
target_link_libraries(test "-ffreestanding -O0 -nostdlib -pie -Wl,-E,-shared")

set_target_properties(test PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(test PROPERTIES VERSION 1.0)
set_target_properties(test PROPERTIES SOVERSION 1)

install(TARGETS test
    LIBRARY DESTINATION sysroot/lib)