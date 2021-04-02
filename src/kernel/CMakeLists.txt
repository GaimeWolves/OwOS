add_subdirectory(arch/${ARCH})

set(HEAP_SOURCES
	include/heap/kmalloc.hpp
	heap/BitmapHeap.hpp
	heap/BitmapHeap.cpp
	heap/kmalloc.cpp
)

set(KERNEL_SOURCES
	include/multiboot.h
	include/IO.hpp
	include/tests.hpp
	include/arch/processor.hpp
	include/crt/icxxabi.hpp
	include/memory/MemoryManager.hpp
	include/memory/MultibootMap.hpp
	include/memory/Pages.hpp
	include/vga/textmode.hpp
	include/libk/kassert.hpp
	include/libk/kstdio.hpp
	include/libk/kctype.hpp
	include/libk/kmath.hpp
	include/libk/kstdarg.hpp
	include/libk/kstring.hpp
	include/libk/kutility.hpp
	include/libk/kvector.hpp
	early_entry.cpp
	entry.cpp
	crt/icxxabi.cpp
	memory/MemoryManager.cpp
	memory/MultibootMap.cpp
	vga/textmode.cpp
	libk/kassert.cpp
	libk/kstdio.cpp
	libk/kctype.cpp
	libk/kstring.cpp
	libk/printf.hpp
	tests/test_crtx.cpp
	tests/test_heap.cpp
	tests/test_printf.cpp
)

set(SOURCES
	${KERNEL_SOURCES}
	${ARCH_SOURCES}
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-ffreestanding -Og -g3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)
add_link_options(LINKER:-T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker.ld -ffreestanding -Og -g -nostdlib)

add_library(kernel_heap STATIC ${HEAP_SOURCES})
target_include_directories(kernel_heap PRIVATE include)

add_executable(kernel ${SOURCES})

target_link_libraries(kernel kernel_heap gcc)
target_include_directories(kernel PRIVATE include)

if (DEBUG_KERNEL)
	target_compile_definitions(kernel PRIVATE _DEBUG=1)
endif(DEBUG_KERNEL)

install(TARGETS kernel
		RUNTIME DESTINATION sysroot/boot)