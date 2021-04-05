set(ARCH_INCLUDES
    include/arch/i686/io.hpp
    include/arch/i686/gdt.hpp
    include/arch/i686/interrupts.hpp
    include/arch/i686/pic.hpp
    PARENT_SCOPE
)

set(ARCH_SOURCES
    arch/i686/boot.S
    arch/i686/processor.cpp
    arch/i686/interrupts.cpp
    arch/i686/pic.cpp
    PARENT_SCOPE
)