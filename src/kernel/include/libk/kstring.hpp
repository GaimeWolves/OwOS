#ifndef KSTRING_H
#define KSTRING_H 1

#include <stdint.h>
#include <stddef.h>

extern "C"
{
    size_t strlen(const char *str);
    char *strrev(char *str);
    int strcmp(const char *lhs, const char *rhs);

    void *memset(void *dest, int ch, size_t count);
}

#endif // KSTRING_H