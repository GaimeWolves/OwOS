#pragma once

#include <bits/guards.h>

__LIBC_BEGIN_DECLS

_Noreturn void __assertion_failed(const char *, const char *, unsigned, const char *);

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/assert.html
// Forcing a definition of the name NDEBUG {...} ahead of the #include <assert.h> statement, shall stop assertions from being compiled into the program.
#ifdef NDEBUG
#	define assert(condition) ((void)0)
#else
#	define assert(condition) ((condition) ? (void)0 : __assertion_failed(#condition, __FILE__, __LINE__, __func__))
#endif

__LIBC_END_DECLS
