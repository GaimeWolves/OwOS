#pragma once

#include <bits/guards.h>

#define LC_CTYPE 1

__LIBC_BEGIN_DECLS

char *setlocale(int category, const char *locale);

__LIBC_END_DECLS
