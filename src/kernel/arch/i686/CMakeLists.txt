set(ARCH_SOURCES
    arch/i686/boot.S
    PARENT_SCOPE
)

add_library(crti OBJECT crti.S)
add_library(crtn OBJECT crtn.S)