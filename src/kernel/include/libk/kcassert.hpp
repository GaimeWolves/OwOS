#pragma once

#include <common_attributes.h>

__noreturn void __assertion_failed(const char *condition, const char *file, unsigned line, const char *function);

#ifdef NDEBUG
#	define assert(condition) ((void)0)
#else
#	define assert(condition) (static_cast<bool>(condition) ? (void)0 : __assertion_failed(#    condition, __FILE__, __LINE__, __PRETTY_FUNCTION__))
#endif
