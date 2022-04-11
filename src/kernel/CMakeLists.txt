add_subdirectory(arch/${ARCH})

set(HEAP_SOURCES
    include/libk/kcmalloc.hpp
    heap/BitmapHeap.hpp
    heap/BitmapHeap.cpp
    heap/kcmalloc.cpp)

set(LIBK_INCLUDES
    include/libk/katomic.hpp
    include/libk/kcassert.hpp
    include/libk/kcstdio.hpp
    include/libk/kcctype.hpp
    include/libk/kmath.hpp
    include/libk/kcstdarg.hpp
    include/libk/kcstring.hpp
    include/libk/kutility.hpp
    include/libk/kvector.hpp
    include/libk/kstring.hpp
    include/libk/kfunctional.hpp
    include/libk/ktype_traits.hpp
    include/libk/kiterator.hpp
    include/libk/__iterators.hpp
    include/libk/AVLTree.hpp
    include/libk/kstack.hpp
    include/libk/kshared_ptr.hpp
    include/libk/Concurrent.hpp)

set(LIBK_SOURCES
    libk/kcassert.cpp
    libk/kcstdio.cpp
    libk/kcctype.cpp
    libk/kcstring.cpp
    libk/printf.hpp)

set(KERNEL_INCLUDES
    include/common_attributes.h
    include/multiboot.h
    include/panic.hpp
    include/tests.hpp
    include/arch/io.hpp
    include/arch/memory.hpp
    include/arch/interrupts.hpp
    include/arch/stack_tracing.hpp
    include/crt/icxxabi.hpp
    include/devices/SerialDevice.hpp
    include/firmware/acpi/Parser.hpp
    include/firmware/BIOS.hpp
    include/memory/PhysicalMemoryManager.hpp
    include/memory/VirtualMemoryManager.hpp
    include/memory/MultibootMap.hpp
    include/pci/definitions.hpp
    include/pci/pci.hpp
    include/vga/textmode.hpp
    include/interrupts/InterruptController.hpp
    include/interrupts/InterruptHandler.hpp
    include/interrupts/InterruptManager.hpp
    include/interrupts/LAPIC.hpp
    include/interrupts/PIC.hpp
    include/interrupts/UnhandledInterruptHandler.hpp
    include/interrupts/definitions.hpp
    include/interrupts/IOAPIC.hpp
    include/interrupts/IRQHandler.hpp
    include/interrupts/SharedIRQHandler.hpp
    include/interrupts/forward.hpp
    include/time/Timer.hpp
    include/time/EventManager.hpp
    include/time/PIT.hpp
    include/arch/smp.hpp
    include/locking/Mutex.hpp
    include/arch/Processor.hpp
    include/processes/definitions.hpp
    include/arch/process.hpp
    include/processes/CoreScheduler.hpp
    include/arch/definitions.hpp
    include/processes/GlobalScheduler.hpp
    include/syscall/SyscallDispatcher.hpp
    include/syscall/syscalls.hpp
    include/processes/Process.hpp
    include/filesystem/File.hpp
    include/filesystem/FileSystem.hpp
    include/devices/Device.hpp
    include/devices/CharacterDevice.hpp
    include/filesystem/VirtualFileSystem.hpp
    include/filesystem/FileContext.hpp
    include/filesystem/definitions.hpp)

set(KERNEL_SOURCES
    early_entry.cpp
    entry.cpp
    panic.cpp
    arch/stack_tracing.cpp
    crt/icxxabi.cpp
    devices/SerialDevice.cpp
    firmware/acpi/Parser.cpp
    firmware/BIOS.cpp
    memory/MultibootMap.cpp
    memory/PhysicalMemoryManager.cpp
    memory/VirtualMemoryManager.cpp
    pci/pci.cpp
    vga/textmode.cpp
    tests/test_crtx.cpp
    tests/test_heap.cpp
    tests/test_printf.cpp
    tests/test_vmm.cpp
    interrupts/LAPIC.cpp
    interrupts/InterruptHandler.cpp
    interrupts/PIC.cpp
    interrupts/InterruptManager.cpp
    interrupts/IOAPIC.cpp
    time/PIT.cpp
    time/EventManager.cpp
    processes/CoreScheduler.cpp
    locking/Mutex.cpp
    arch/Processor.cpp
    processes/GlobalScheduler.cpp
    syscall/SyscallDispatcher.cpp
    processes/Process.cpp
    filesystem/FileContext.cpp)

set(SOURCES
    ${KERNEL_INCLUDES}
    ${ARCH_INCLUDES}
    ${LIBK_INCLUDES}
    ${KERNEL_SOURCES}
    ${ARCH_SOURCES}
    ${LIBK_SOURCES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CFLAGS -ffreestanding -O0 -g3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fdebug-prefix-map=${CMAKE_SOURCE_DIR}= -fno-omit-frame-pointer)

add_compile_options(${CFLAGS})
add_link_options(LINKER:-T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker.ld -ffreestanding -Og -g -nostdlib)

add_library(kernel_heap STATIC ${HEAP_SOURCES})
target_include_directories(kernel_heap PRIVATE include)

add_executable(kernel ${SOURCES})

target_link_libraries(kernel kernel_heap gcc)
target_include_directories(kernel PRIVATE include)

target_compile_definitions(kernel_heap PRIVATE ARCH_${ARCH})
target_compile_definitions(kernel PRIVATE ARCH_${ARCH})

if (DEBUG_KERNEL)
    target_compile_definitions(kernel PRIVATE _DEBUG=1)
endif (DEBUG_KERNEL)

install(TARGETS kernel
    RUNTIME DESTINATION sysroot/boot)
