#ifndef KSTDIO_H
#define KSTDIO_H 1

extern "C"
{
    void kputc(const char ch);
    void kputs(const char *str);

    void print_init_msg(bool failed, const char *msg);
}

#endif