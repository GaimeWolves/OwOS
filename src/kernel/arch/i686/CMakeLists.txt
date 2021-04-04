set(ARCH_INCLUDES
    include/arch/i686/io.hpp
    include/arch/i686/gdt.hpp
    PARENT_SCOPE
)

set(ARCH_SOURCES
    arch/i686/boot.S
    arch/i686/processor.cpp
    PARENT_SCOPE
)