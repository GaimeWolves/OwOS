add_subdirectory(arch/${ARCH})

set(HEAP_SOURCES
    heap/BitmapHeap.cpp
    heap/BitmapHeap.hpp
    heap/kcmalloc.cpp
    include/libk/kcmalloc.hpp)

set(LIBK_INCLUDES
    include/libk/__iterators.hpp
    include/libk/ArrayView.hpp
    include/libk/AVLTree.hpp
    include/libk/CircularBuffer.hpp
    include/libk/Concurrent.hpp
    include/libk/GUID.hpp
    include/libk/katomic.hpp
    include/libk/kcassert.hpp
    include/libk/kcctype.hpp
    include/libk/kcstdarg.hpp
    include/libk/kcstdio.hpp
    include/libk/kcstring.hpp
    include/libk/kfunctional.hpp
    include/libk/kiterator.hpp
    include/libk/klist.hpp
    include/libk/kmath.hpp
    include/libk/kmemory.hpp
    include/libk/kshared_ptr.hpp
    include/libk/kstack.hpp
    include/libk/kstring.hpp
    include/libk/ktype_traits.hpp
    include/libk/kutility.hpp
    include/libk/kvector.hpp
    include/libk/printf.hpp
    include/libk/StringView.hpp
    include/libk/ErrorOr.hpp
    include/libk/kcstdlib.hpp)

set(LIBK_SOURCES
    libk/kcassert.cpp
    libk/kcctype.cpp
    libk/kcstdio.cpp
    libk/kcstring.cpp
    libk/printf.cpp
    libk/kcstdlib.cpp)

set(KERNEL_INCLUDES
    include/arch/definitions.hpp
    include/arch/i686/msr.hpp
    include/arch/interrupts.hpp
    include/arch/io.hpp
    include/arch/memory.hpp
    include/arch/process.hpp
    include/arch/Processor.hpp
    include/arch/smp.hpp
    include/arch/stack_tracing.hpp
    include/common_attributes.h
    include/crt/icxxabi.hpp
    include/devices/BlockDevice.hpp
    include/devices/CharacterDevice.hpp
    include/devices/Device.hpp
    include/devices/FramebufferDevice.hpp
    include/devices/KeyboardDevice.hpp
    include/devices/KeyCode.hpp
    include/devices/PS2KeyboardDevice.hpp
    include/devices/SerialDevice.hpp
    include/elf/definitions.hpp
    include/elf/elf.hpp
    include/filesystem/definitions.hpp
    include/filesystem/Ext2FileSystem.hpp
    include/filesystem/File.hpp
    include/filesystem/FileContext.hpp
    include/filesystem/FileSystem.hpp
    include/filesystem/VirtualFileSystem.hpp
    include/firmware/acpi/Parser.hpp
    include/firmware/BIOS.hpp
    include/interrupts/definitions.hpp
    include/interrupts/forward.hpp
    include/interrupts/InterruptController.hpp
    include/interrupts/InterruptHandler.hpp
    include/interrupts/InterruptManager.hpp
    include/interrupts/IOAPIC.hpp
    include/interrupts/IRQHandler.hpp
    include/interrupts/LAPIC.hpp
    include/interrupts/PIC.hpp
    include/interrupts/SharedIRQHandler.hpp
    include/interrupts/UnhandledInterruptHandler.hpp
    include/libk/srmw_queue.hpp
    include/locking/Mutex.hpp
    include/logging/logger.hpp
    include/memory/MultibootMap.hpp
    include/memory/PhysicalMemoryManager.hpp
    include/memory/VirtualMemoryManager.hpp
    include/multiboot.h
    include/panic.hpp
    include/pci/definitions.hpp
    include/pci/pci.hpp
    include/processes/CoreScheduler.hpp
    include/processes/definitions.hpp
    include/processes/GlobalScheduler.hpp
    include/processes/Process.hpp
    include/storage/ata/AHCICommandSlot.hpp
    include/storage/ata/AHCIController.hpp
    include/storage/ata/AHCIManager.hpp
    include/storage/ata/AHCIPort.hpp
    include/storage/ata/definitions.hpp
    include/storage/definitions.hpp
    include/storage/GPT.hpp
    include/storage/PartitionDevice.hpp
    include/storage/StorageDevice.hpp
    include/syscall/SyscallDispatcher.hpp
    include/syscall/syscalls.hpp
    include/tests.hpp
    include/time/EventManager.hpp
    include/time/PIT.hpp
    include/time/Timer.hpp
    include/tty/Console.hpp
    include/tty/definitions.hpp
    include/tty/FramebufferConsole.hpp
    include/tty/psf.hpp
    include/tty/TTY.hpp
    include/vga/textmode.hpp
    include/tty/Terminal.hpp
    include/tty/ANSIColor.hpp
    include/tty/VirtualConsole.hpp
    include/filesystem/FileSystemCache.hpp)

set(KERNEL_SOURCES
    arch/Processor.cpp
    arch/stack_tracing.cpp
    crt/icxxabi.cpp
    devices/FramebufferDevice.cpp
    devices/KeyboardDevice.cpp
    devices/PS2KeyboardDevice.cpp
    devices/SerialDevice.cpp
    early_entry.cpp
    elf/elf.cpp
    entry.cpp
    filesystem/Ext2FileSystem.cpp
    filesystem/File.cpp
    filesystem/FileContext.cpp
    filesystem/VirtualFileSystem.cpp
    firmware/acpi/Parser.cpp
    firmware/BIOS.cpp
    interrupts/InterruptHandler.cpp
    interrupts/InterruptManager.cpp
    interrupts/IOAPIC.cpp
    interrupts/LAPIC.cpp
    interrupts/PIC.cpp
    locking/Mutex.cpp
    logging/logger.cpp
    memory/MultibootMap.cpp
    memory/PhysicalMemoryManager.cpp
    memory/VirtualMemoryManager.cpp
    panic.cpp
    pci/pci.cpp
    processes/CoreScheduler.cpp
    processes/GlobalScheduler.cpp
    processes/Process.cpp
    storage/ata/AHCICommandSlot.cpp
    storage/ata/AHCIController.cpp
    storage/ata/AHCIManager.cpp
    storage/ata/AHCIPort.cpp
    storage/GPT.cpp
    storage/PartitionDevice.cpp
    storage/StorageDevice.cpp
    syscall/chdir.cpp
    syscall/getcwd.cpp
    syscall/getdents.cpp
    syscall/close.cpp
    syscall/ioctl.cpp
    syscall/mmap.cpp
    syscall/munmap.cpp
    syscall/open.cpp
    syscall/read.cpp
    syscall/stat.cpp
    syscall/SyscallDispatcher.cpp
    syscall/write.cpp
    tests/definitions.hpp
    tests/test_crtx.cpp
    tests/test_heap.cpp
    tests/test_printf.cpp
    tests/test_vmm.cpp
    time/EventManager.cpp
    time/PIT.cpp
    tty/FramebufferConsole.cpp
    tty/psf.cpp
    tty/TTY.cpp
    vga/textmode.cpp
    tty/Terminal.cpp
    syscall/exit.cpp
    syscall/exec.cpp
    syscall/fork.cpp
    syscall/waitpid.cpp
    crt/ssp.cpp
    crt/ubsan.cpp
    filesystem/FileSystemCache.cpp)

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

add_custom_command(
    OUTPUT font_out.S
    COMMAND sed -e "s,@file@,${CMAKE_CURRENT_SOURCE_DIR}/tty/font.psf,g" < ${CMAKE_CURRENT_SOURCE_DIR}/tty/font_in.S > font_out.S
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tty/font.psf ${CMAKE_CURRENT_SOURCE_DIR}/tty/font_in.S
    VERBATIM
)

add_executable(kernel ${SOURCES} font_out.S)

set(CFLAGS -ffreestanding -O0 -g3 -Wall -Wextra -Werror -fno-exceptions -fno-threadsafe-statics -fstack-protector-all -fno-rtti -fdebug-prefix-map=${CMAKE_SOURCE_DIR}= -fno-omit-frame-pointer -fsanitize=undefined)
set(LFLAGS LINKER:-T ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ARCH}/linker.ld -ffreestanding -O0 -g3 -nostdlib)

target_compile_options(kernel PRIVATE ${CFLAGS})
target_compile_options(kernel_heap PRIVATE ${CFLAGS})
target_link_options(kernel PRIVATE ${LFLAGS})
target_link_options(kernel_heap PRIVATE  ${LFLAGS})

set_source_files_properties(arch/i686/interrupts.cpp PROPERTIES COMPILE_FLAGS -fno-stack-protector)
set_source_files_properties(crt/ssp.cpp PROPERTIES COMPILE_FLAGS -fno-stack-protector)

target_link_libraries(kernel kernel_heap gcc)
target_include_directories(kernel PRIVATE include)

# Enable this when our libc headers are more mature or find a better solution to get at sys/arch/i386/syscall.h
target_include_directories(kernel PRIVATE ${CMAKE_SOURCE_DIR}/src/userland/libc)

target_compile_definitions(kernel_heap PRIVATE ARCH_${ARCH})
target_compile_definitions(kernel PRIVATE ARCH_${ARCH})

target_compile_definitions(kernel_heap PRIVATE __OWOS_KERNEL)
target_compile_definitions(kernel PRIVATE __OWOS_KERNEL)

target_compile_definitions(kernel_heap PRIVATE __LIBC_KEEP_DEFS)
target_compile_definitions(kernel PRIVATE __LIBC_KEEP_DEFS)

if (DEBUG_KERNEL)
    target_compile_definitions(kernel PRIVATE _DEBUG=1)
endif (DEBUG_KERNEL)

install(TARGETS kernel
    RUNTIME DESTINATION sysroot/boot)
