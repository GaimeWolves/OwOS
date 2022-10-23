#include <ctype.h>

int isalnum(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_UPPER | __CT_LOWER | __CT_DIGIT); }
int isalpha(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_UPPER | __CT_LOWER); }
int iscntrl(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_CNTRL); }
int isgraph(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_PUNCT | __CT_UPPER | __CT_LOWER | __CT_DIGIT); }
int islower(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_LOWER); }
int isupper(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_UPPER); }
int isprint(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_PUNCT | __CT_UPPER | __CT_LOWER | __CT_DIGIT | __CT_SPACE); }
int ispunct(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_PUNCT); }
int isspace(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_WHITE); }
int isdigit(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_DIGIT); }
int isxdigit(int c) { return __ctype_lookup[(unsigned char)(c)] & (__CT_DIGIT | __CT_HEXDG); }

int tolower(int c)
{
	if (__ctype_lookup[c] & __CT_UPPER)
		return c + ('a' - 'A');
	else
		return c;
}

int toupper(int c)
{
	if (__ctype_lookup[c] & __CT_LOWER)
		return c - ('a' - 'A');
	else
		return c;
}

int isascii(int c) { return c <= 0x7F; }
int toascii(int c) { return c & 0x7F; }

unsigned char const __ctype_lookup[256] = {
    __CT_CNTRL,              // 00 (NUL)
    __CT_CNTRL,              // 01 (SOH)
    __CT_CNTRL,              // 02 (STX)
    __CT_CNTRL,              // 03 (ETX)
    __CT_CNTRL,              // 04 (EOT)
    __CT_CNTRL,              // 05 (ENQ)
    __CT_CNTRL,              // 06 (ACK)
    __CT_CNTRL,              // 07 (BEL)
    __CT_CNTRL,              // 08 (BS)
    __CT_WHITE + __CT_CNTRL, // 09 (HT)
    __CT_WHITE + __CT_CNTRL, // 0A (LF)
    __CT_WHITE + __CT_CNTRL, // 0B (VT)
    __CT_WHITE + __CT_CNTRL, // 0C (FF)
    __CT_WHITE + __CT_CNTRL, // 0D (CR)
    __CT_CNTRL,              // 0E (SI)
    __CT_CNTRL,              // 0F (SO)
    __CT_CNTRL,              // 10 (DLE)
    __CT_CNTRL,              // 11 (DC1)
    __CT_CNTRL,              // 12 (DC2)
    __CT_CNTRL,              // 13 (DC3)
    __CT_CNTRL,              // 14 (DC4)
    __CT_CNTRL,              // 15 (NAK)
    __CT_CNTRL,              // 16 (SYN)
    __CT_CNTRL,              // 17 (ETB)
    __CT_CNTRL,              // 18 (CAN)
    __CT_CNTRL,              // 19 (EM)
    __CT_CNTRL,              // 1A (SUB)
    __CT_CNTRL,              // 1B (ESC)
    __CT_CNTRL,              // 1C (FS)
    __CT_CNTRL,              // 1D (GS)
    __CT_CNTRL,              // 1E (RS)
    __CT_CNTRL,              // 1F (US)
    __CT_SPACE + __CT_WHITE, // 20 Space
    __CT_PUNCT,              // 21 !
    __CT_PUNCT,              // 22 "
    __CT_PUNCT,              // 23 #
    __CT_PUNCT,              // 24 $
    __CT_PUNCT,              // 25 %
    __CT_PUNCT,              // 26 &
    __CT_PUNCT,              // 27 '
    __CT_PUNCT,              // 28 (
    __CT_PUNCT,              // 29 )
    __CT_PUNCT,              // 2A *
    __CT_PUNCT,              // 2B +
    __CT_PUNCT,              // 2C ,
    __CT_PUNCT,              // 2D -
    __CT_PUNCT,              // 2E .
    __CT_PUNCT,              // 2F /
    __CT_DIGIT + __CT_HEXDG, // 30 0
    __CT_DIGIT + __CT_HEXDG, // 31 1
    __CT_DIGIT + __CT_HEXDG, // 32 2
    __CT_DIGIT + __CT_HEXDG, // 33 3
    __CT_DIGIT + __CT_HEXDG, // 34 4
    __CT_DIGIT + __CT_HEXDG, // 35 5
    __CT_DIGIT + __CT_HEXDG, // 36 6
    __CT_DIGIT + __CT_HEXDG, // 37 7
    __CT_DIGIT + __CT_HEXDG, // 38 8
    __CT_DIGIT + __CT_HEXDG, // 39 9
    __CT_PUNCT,              // 3A :
    __CT_PUNCT,              // 3B ;
    __CT_PUNCT,              // 3C <
    __CT_PUNCT,              // 3D =
    __CT_PUNCT,              // 3E >
    __CT_PUNCT,              // 3F ?
    __CT_PUNCT,              // 40 @
    __CT_UPPER + __CT_HEXDG, // 41 A
    __CT_UPPER + __CT_HEXDG, // 42 B
    __CT_UPPER + __CT_HEXDG, // 43 C
    __CT_UPPER + __CT_HEXDG, // 44 D
    __CT_UPPER + __CT_HEXDG, // 45 E
    __CT_UPPER + __CT_HEXDG, // 46 F
    __CT_UPPER,              // 47 G
    __CT_UPPER,              // 48 H
    __CT_UPPER,              // 49 I
    __CT_UPPER,              // 4A J
    __CT_UPPER,              // 4B K
    __CT_UPPER,              // 4C L
    __CT_UPPER,              // 4D M
    __CT_UPPER,              // 4E N
    __CT_UPPER,              // 4F O
    __CT_UPPER,              // 50 P
    __CT_UPPER,              // 51 Q
    __CT_UPPER,              // 52 R
    __CT_UPPER,              // 53 S
    __CT_UPPER,              // 54 T
    __CT_UPPER,              // 55 U
    __CT_UPPER,              // 56 V
    __CT_UPPER,              // 57 W
    __CT_UPPER,              // 58 X
    __CT_UPPER,              // 59 Y
    __CT_UPPER,              // 5A Z
    __CT_PUNCT,              // 5B [
    __CT_PUNCT,              // 5C (\)
    __CT_PUNCT,              // 5D ]
    __CT_PUNCT,              // 5E ^
    __CT_PUNCT,              // 5F _
    __CT_PUNCT,              // 60 `
    __CT_LOWER + __CT_HEXDG, // 61 a
    __CT_LOWER + __CT_HEXDG, // 62 b
    __CT_LOWER + __CT_HEXDG, // 63 c
    __CT_LOWER + __CT_HEXDG, // 64 d
    __CT_LOWER + __CT_HEXDG, // 65 e
    __CT_LOWER + __CT_HEXDG, // 66 f
    __CT_LOWER,              // 67 g
    __CT_LOWER,              // 68 h
    __CT_LOWER,              // 69 i
    __CT_LOWER,              // 6A j
    __CT_LOWER,              // 6B k
    __CT_LOWER,              // 6C l
    __CT_LOWER,              // 6D m
    __CT_LOWER,              // 6E n
    __CT_LOWER,              // 6F o
    __CT_LOWER,              // 70 p
    __CT_LOWER,              // 71 q
    __CT_LOWER,              // 72 r
    __CT_LOWER,              // 73 s
    __CT_LOWER,              // 74 t
    __CT_LOWER,              // 75 u
    __CT_LOWER,              // 76 v
    __CT_LOWER,              // 77 w
    __CT_LOWER,              // 78 x
    __CT_LOWER,              // 79 y
    __CT_LOWER,              // 7A z
    __CT_PUNCT,              // 7B {
    __CT_PUNCT,              // 7C |
    __CT_PUNCT,              // 7D }
    __CT_PUNCT,              // 7E ~
    __CT_CNTRL,              // 7F (DEL)
                             // The rest is zero
};