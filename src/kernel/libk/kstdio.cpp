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

void print_check_msg(bool ok, const char *msg)
{
    kputc('[');

    if (ok)
    {
        Textmode::set_color(Textmode::Color::GREEN, Textmode::Color::BLACK);
        kputs("  OK  ");
    }
    else
    {
        Textmode::set_color(Textmode::Color::RED, Textmode::Color::BLACK);
        kputs("FAILED");
    }

    Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);
    kputc(']');
    kputc(' ');

    kputs(msg);

    kputc('\n');
}

void print_debug_msg(const char *msg)
{
    kputc('[');

    Textmode::set_color(Textmode::Color::MAGENTA, Textmode::Color::BLACK);
    kputs(" DEBUG");
    Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);

    kputc(']');
    kputc(' ');

    kputs(msg);

    kputc('\n');
}

void print_test_msg(const char *msg)
{
    kputc('[');

    Textmode::set_color(Textmode::Color::LIGHT_GREEN, Textmode::Color::BLACK);
    kputs(" TEST ");
    Textmode::set_color(Textmode::Color::WHITE, Textmode::Color::BLACK);

    kputc(']');
    kputc(' ');

    kputs(msg);

    kputc('\n');
}