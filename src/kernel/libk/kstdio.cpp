#include <libk/kstdio.hpp>

#include <vga/textmode.hpp>

using namespace Kernel::VGA;

void kputc(const char ch)
{
    Textmode::putc(ch);
}

void kputs(const char *str)
{
    Textmode::puts(str);
}

void print_init_msg(bool failed, const char *msg)
{
    kputc('[');

    if (failed)
    {
        Textmode::set_color(Textmode::Color::RED, Textmode::Color::BLACK);
        kputs("FAILED");
    }
    else
    {
        Textmode::set_color(Textmode::Color::GREEN, Textmode::Color::BLACK);
        kputs("  OK  ");
    }

    Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);
    kputc(']');
    kputc(' ');

    kputs(msg);

    kputc('\n');
}