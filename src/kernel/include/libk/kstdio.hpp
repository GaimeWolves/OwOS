#ifndef KSTDIO_H
#define KSTDIO_H 1

#include <libk/kstdarg.hpp>

extern "C"
{
    void kputc(const char ch);
    void kputs(const char *str);

    void print_check_msg(bool ok, const char *msg);
    void print_debug_msg(const char *msg);
    void print_test_msg(const char *msg);

    void kprintf(const char *fmt, ...);
    void ksprintf(char *buffer, const char *fmt, ...);

    void kvprintf(const char *fmt, va_list ap);
    void kvsprintf(char *buffer, const char *fmt, va_list ap);
}

namespace Kernel::LibK
{

    void printf_check_msg(bool ok, const char *msg, ...);
    void printf_debug_msg(const char *msg, ...);
    void printf_test_msg(const char *msg, ...);

} // namespace Kernel::LibK

#endif // KSTDIO_H