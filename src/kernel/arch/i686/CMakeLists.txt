set(ARCH_INCLUDES
    include/arch/i686/io.hpp
    include/arch/i686/gdt.hpp
    include/arch/i686/interrupts.hpp
    include/arch/i686/memory.hpp
    include/arch/i686/Processor.hpp
    include/arch/i686/process.hpp
    PARENT_SCOPE
)

set(ARCH_SOURCES
    arch/i686/boot.S
    arch/i686/bootAP.S
    arch/i686/Processor.cpp
    arch/i686/interrupts.cpp
    arch/i686/memory.cpp
    arch/i686/stack_tracing.cpp
    arch/i686/smp.cpp
    arch/i686/gdt.cpp
    arch/i686/spinlock.cpp
    PARENT_SCOPE
)

set_source_files_properties(arch/i686/interrupts.cpp PROPERTIES COMPILE_FLAGS -fno-stack-protector)
