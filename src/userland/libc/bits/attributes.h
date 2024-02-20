#pragma once

#if defined(__cplusplus)
#	define __deprecated [[deprecated]]
#elif defined(__GNUC__) || defined(__clang__)
#	define __deprecated __attribute__((__deprecated__))
#elif defined(_MSC_VER)
#	define __deprecated __declspec(deprecated)
#else
#	define __deprecated
#endif

#if defined(__cplusplus) || __STDC_VERSION__ > 201710L
#undef __noreturn
#define __noreturn [[noreturn]]
#else
#include <stdnoreturn.h>
#undef __noreturn
#define __noreturn _Noreturn
#endif
