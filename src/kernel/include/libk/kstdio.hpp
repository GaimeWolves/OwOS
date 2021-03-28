#ifndef KSTDIO_H
#define KSTDIO_H 1

extern "C"
{
    void kputc(const char ch);
    void kputs(const char *str);

    void print_check_msg(bool ok, const char *msg);
    void print_debug_msg(const char *msg);
    void print_test_msg(const char *msg);
}

#endif