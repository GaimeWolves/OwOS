#ifndef VGA_TEXTMODE_H
#define VGA_TEXTMODE_H 1

#include <stddef.h>
#include <stdint.h>

namespace Kernel::VGA::Textmode
{
    enum class Color : uint8_t
    {
        BLACK = 0,
        BLUE = 1,
        GREEN = 2,
        CYAN = 3,
        RED = 4,
        MAGENTA = 5,
        BROWN = 6,
        LIGHT_GREY = 7,
        DARK_GREY = 8,
        LIGHT_BLUE = 9,
        LIGHT_GREEN = 10,
        LIGHT_CYAN = 11,
        LIGHT_RED = 12,
        LIGHT_MAGENTA = 13,
        LIGHT_BROWN = 14,
        WHITE = 15,
    };

    void init();

    void putc(const char ch);
    void puts(const char *str);

    void set_color(Color fg, Color bg);
} // namespace Kernel::VGA::Textmode

#endif // VGA_TEXTMODE_H