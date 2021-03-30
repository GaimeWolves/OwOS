#ifndef CTYPE_H
#define CTYPE_H 1

#include <stdint.h>

#define CT_UPPER 0b00000001 // Upper case
#define CT_LOWER 0b00000010 // Lower case
#define CT_DIGIT 0b00000100 // Digit
#define CT_CNTRL 0b00001000 // Control character
#define CT_PUNCT 0b00010000 // Punctuation
#define CT_WHITE 0b00100000 // Whitespace
#define CT_HEXDG 0b01000000 // Hex digit
#define CT_SPACE 0b10000000 // Space

// Lookup table holding the flags for each char
extern uint8_t __ctype_lookup[];

inline __attribute__((always_inline)) int isalnum(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_UPPER | CT_LOWER | CT_DIGIT); }
inline __attribute__((always_inline)) int isalpha(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_UPPER | CT_LOWER); }
inline __attribute__((always_inline)) int iscntrl(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_CNTRL); }
inline __attribute__((always_inline)) int isgraph(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_PUNCT | CT_UPPER | CT_LOWER | CT_DIGIT); }
inline __attribute__((always_inline)) int islower(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_LOWER); }
inline __attribute__((always_inline)) int isupper(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_UPPER); }
inline __attribute__((always_inline)) int isprint(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_PUNCT | CT_UPPER | CT_LOWER | CT_DIGIT | CT_SPACE); }
inline __attribute__((always_inline)) int ispunct(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_PUNCT); }
inline __attribute__((always_inline)) int isspace(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_WHITE); }
inline __attribute__((always_inline)) int isdigit(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_DIGIT); }
inline __attribute__((always_inline)) int isxdigit(int c) { return __ctype_lookup[(uint8_t)(c)] & (CT_DIGIT | CT_HEXDG); }

#define isascii(c) ((unsigned)(c) <= 0x7F)
#define toascii(c) ((unsigned)(c)&0x7F)

inline __attribute__((always_inline)) int tolower(int c)
{
    if (__ctype_lookup[c] & CT_UPPER)
        return c + ('a' - 'A');
    else
        return c;
}

inline __attribute__((always_inline)) int toupper(int c)
{
    if (__ctype_lookup[c] & CT_LOWER)
        return c - ('a' - 'A');
    else
        return c;
}

#ifndef CTYPE_H_KEEP_DEFS
#undef CT_UPPER
#undef CT_LOWER
#undef CT_DIGIT
#undef CT_CNTRL
#undef CT_PUNCT
#undef CT_WHITE
#undef CT_HEXDG
#undef CT_SPACE
#endif

#endif // CTYPE_H
