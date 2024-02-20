#pragma once

#include <bits/guards.h>
#include <stdio.h>
#include <unistd.h>

__LIBC_BEGIN_DECLS

#ifdef __LIBC_DEBUG
extern int __in_trace;
#	define TRACE(fmt, ...)                                             \
		do                                                              \
		{                                                               \
			static char __trace_buf[256];                               \
			if (!__in_trace)                                            \
			{                                                           \
				__in_trace = 1;                                         \
				int __n = snprintf(__trace_buf, 256, fmt, __VA_ARGS__); \
				write(0, __trace_buf, __n);                             \
				__in_trace = 0;                                         \
			}                                                           \
		} while (0)
#else
#	define TRACE(fmt, ...) ((void)(0))
#endif

__LIBC_END_DECLS
