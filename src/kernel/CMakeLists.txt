add_subdirectory(arch/${ARCH})

set(KERNEL_SOURCES
    include/multiboot
    entry.cpp
)

set(SOURCES
    ${KERNEL_SOURCES}
    ${ARCH_SOURCES}
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-ffreestanding -Og -g -Wall -Wextra -Werror -fno-exceptions -fno-rtti)
add_link_options(LINKER:-T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker.ld -ffreestanding -Og -g -nostdlib -lgcc)

add_executable(kernel ${SOURCES})

target_include_directories(kernel PRIVATE include)

install(TARGETS kernel
        RUNTIME DESTINATION sysroot/boot)