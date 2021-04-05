add_subdirectory(arch/${ARCH})

set(HEAP_SOURCES
	include/libk/kcmalloc.hpp
	heap/BitmapHeap.hpp
	heap/BitmapHeap.cpp
	heap/kcmalloc.cpp
)

set(LIBK_INCLUDES
	include/libk/kcassert.hpp
	include/libk/kcstdio.hpp
	include/libk/kcctype.hpp
	include/libk/kmath.hpp
	include/libk/kcstdarg.hpp
	include/libk/kcstring.hpp
	include/libk/kutility.hpp
	include/libk/kvector.hpp
)

set(LIBK_SOURCES
	libk/kcassert.cpp
	libk/kcstdio.cpp
	libk/kcctype.cpp
	libk/kcstring.cpp
	libk/printf.hpp
)

set(KERNEL_INCLUDES
	include/multiboot.h
	include/tests.hpp
	include/arch/io.hpp
	include/arch/processor.hpp
	include/crt/icxxabi.hpp
	include/memory/MemoryManager.hpp
	include/memory/MultibootMap.hpp
	include/memory/Pages.hpp
	include/vga/textmode.hpp
)

set(KERNEL_SOURCES
	early_entry.cpp
	entry.cpp
	crt/icxxabi.cpp
	memory/MemoryManager.cpp
	memory/MultibootMap.cpp
	vga/textmode.cpp
	tests/test_crtx.cpp
	tests/test_heap.cpp
	tests/test_printf.cpp
)

set(SOURCES
	${KERNEL_INCLUDES}
	${ARCH_INCLUDES}
	${LIBK_INCLUDES}
	${KERNEL_SOURCES}
	${ARCH_SOURCES}
	${LIBK_SOURCES}
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

target_compile_definitions(kernel PRIVATE ARCH_${ARCH})

if (DEBUG_KERNEL)
	target_compile_definitions(kernel PRIVATE _DEBUG=1)
endif(DEBUG_KERNEL)

install(TARGETS kernel
	RUNTIME DESTINATION sysroot/boot)