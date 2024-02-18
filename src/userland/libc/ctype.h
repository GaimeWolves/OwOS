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

__LIBC_HEADER_BEGIN

extern unsigned char const __ctype_lookup[];

int isalnum(int);
int isalpha(int);
int isascii(int);
int iscntrl(int);
int isdigit(int);
int isxdigit(int);
int isspace(int);
int ispunct(int);
int isprint(int);
int isgraph(int);
int islower(int);
int isupper(int);
int isblank(int);
int toascii(int);
int tolower(int);
int toupper(int);

__LIBC_HEADER_END
