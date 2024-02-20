#pragma once

#include <bits/guards.h>

#define __CT_UPPER 0b00000001
#define __CT_LOWER 0b00000010
#define __CT_DIGIT 0b00000100
#define __CT_CNTRL 0b00001000
#define __CT_PUNCT 0b00010000
#define __CT_WHITE 0b00100000
#define __CT_HEXDG 0b01000000
#define __CT_SPACE 0b10000000

__LIBC_BEGIN_DECLS

extern unsigned char const __ctype_lookup[];

int isalnum(int c);
int isalpha(int c);
int isascii(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int toascii(int c);
int tolower(int c);
int toupper(int c);

__LIBC_END_DECLS
