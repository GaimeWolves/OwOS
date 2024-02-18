#pragma once

#ifndef NDEBUG
#include <stdio.h>
#define DEBUG_PUTS(d, str) debug_puts(d, str)
#define DEBUG_PRINTF(d, fmt, ...) debug_printf(d, fmt, __VA_ARGS__)
#else
#define DEBUG_PUTS(d, str) ((void)0)
#define DEBUG_PRINTF(d, fmt, ...) ((void)0)
#endif

void debug_puts(int depth, const char *str);
void debug_printf(int depth, const char *fmt, ...);
