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
    include/libk/Concurrent.hpp
    include/libk/GUID.hpp
    include/libk/ArrayView.hpp
    include/libk/printf.hpp
    include/libk/StringView.hpp
    include/libk/klist.hpp include/libk/kmemory.hpp)

set(LIBK_SOURCES
    libk/kcassert.cpp
    libk/kcstdio.cpp
    libk/kcctype.cpp
    libk/kcstring.cpp
    libk/printf.cpp)

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
    include/filesystem/definitions.hpp
    include/devices/BlockDevice.hpp
    include/storage/StorageDevice.hpp
    include/storage/PartitionDevice.hpp
    include/storage/ata/AHCIManager.hpp
    include/storage/ata/AHCIController.hpp
    include/storage/ata/AHCIPort.hpp
    include/storage/ata/AHCICommandSlot.hpp
    include/storage/ata/definitions.hpp
    include/storage/GPT.hpp
    include/storage/definitions.hpp
    include/filesystem/Ext2FileSystem.hpp
    include/elf/definitions.hpp
    include/elf/elf.hpp
    include/logging/logger.hpp)

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
    tests/definitions.hpp
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
    filesystem/FileContext.cpp
    storage/ata/AHCIController.cpp
    storage/ata/AHCIManager.cpp
    storage/ata/AHCIPort.cpp
    storage/ata/AHCICommandSlot.cpp
    storage/StorageDevice.cpp
    storage/PartitionDevice.cpp
    storage/GPT.cpp
    filesystem/Ext2FileSystem.cpp
    filesystem/VirtualFileSystem.cpp
    filesystem/File.cpp elf/elf.cpp
    include/libk/srmw_queue.hpp
    logging/logger.cpp)

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

add_library(kernel_heap STATIC ${HEAP_SOURCES})
target_include_directories(kernel_heap PRIVATE include)

add_executable(kernel ${SOURCES})

target_compile_options(kernel PRIVATE -ffreestanding -O0 -g3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fdebug-prefix-map=${CMAKE_SOURCE_DIR}= -fno-omit-frame-pointer)
target_compile_options(kernel_heap PRIVATE -ffreestanding -O0 -g3 -Wall -Wextra -Werror -fno-exceptions -fno-rtti -fdebug-prefix-map=${CMAKE_SOURCE_DIR}= -fno-omit-frame-pointer)
target_link_options(kernel PRIVATE LINKER:-T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker.ld -ffreestanding -Og -g -nostdlib)
target_link_options(kernel_heap PRIVATE LINKER:-T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker.ld -ffreestanding -Og -g -nostdlib)

target_link_libraries(kernel kernel_heap gcc)
target_include_directories(kernel PRIVATE include)

# Enable this when our libc headers are more mature or find a better solution to get at sys/arch/i386/syscall.h
# target_include_directories(kernel PRIVATE ${CMAKE_SOURCE_DIR}/src/userland/libc)

target_compile_definitions(kernel_heap PRIVATE ARCH_${ARCH})
target_compile_definitions(kernel PRIVATE ARCH_${ARCH})

target_compile_definitions(kernel_heap PRIVATE __OWOS_KERNEL)
target_compile_definitions(kernel PRIVATE __OWOS_KERNEL)

if (DEBUG_KERNEL)
    target_compile_definitions(kernel PRIVATE _DEBUG=1)
endif (DEBUG_KERNEL)

install(TARGETS kernel
    RUNTIME DESTINATION sysroot/boot)
