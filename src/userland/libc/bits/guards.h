#pragma once

#ifdef __cplusplus
#	ifndef __LIBC_HEADER_BEGIN
#		define __LIBC_HEADER_BEGIN \
			extern "C"             \
			{
#		define __LIBC_HEADER_END }
#	endif
#else
#	ifndef __LIBC_HEADER_BEGIN
#		define __LIBC_HEADER_BEGIN
#		define __LIBC_HEADER_END
#	endif
#endif