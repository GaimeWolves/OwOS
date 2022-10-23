#pragma once

#include <bits/guards.h>
#include <stdbool.h>

__LIBC_HEADER_BEGIN

__attribute__((noreturn)) void __assertion_failed(const char *, const char *, unsigned, const char *);

#ifdef NDEBUG
#	define assert(condition) ((void)0)
#else
#	define assert(condition) ((bool)(condition) ? (void)0 : __assertion_failed(#    condition, __FILE__, __LINE__, __PRETTY_FUNCTION__))
#endif

__LIBC_HEADER_END