set(ARCH_SOURCES
    arch/i686/boot.S
    arch/i686/IO.cpp
    PARENT_SCOPE
)

add_library(crti OBJECT crti.S)
add_library(crtn OBJECT crtn.S)