#include <libk/kstring.hpp>

extern "C"
{
    size_t strlen(const char *str)
    {
        size_t len = 0;

        while (*(str++))
            len++;

        return len;
    }

    char *strrev(char *str)
    {
        size_t len = strlen(str);

        for (size_t i = 0; i < len / 2; i++)
        {
            char ch = str[i];
            str[i] = str[len - i - 1];
            str[len - i - 1] = ch;
        }

        return str;
    }

    int strcmp(const char *lhs, const char *rhs)
    {
        char c1, c2;

        do
        {
            c1 = *lhs++;
            c2 = *rhs++;
        } while (c1 == c2 && c1);

        return c1 - c2;
    }

    void *memset(void *dest, int ch, size_t count)
    {
        char *p = (char *)dest;
        while (count--)
            *p++ = (char)ch;

        return dest;
    }
}