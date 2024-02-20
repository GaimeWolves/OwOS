#pragma once

#ifdef __cplusplus
#	ifndef __LIBC_BEGIN_DECLS
#		define __LIBC_BEGIN_DECLS \
			extern "C"             \
			{
#		define __LIBC_END_DECLS }
#	endif
#else
#	ifndef __LIBC_BEGIN_DECLS
#		define __LIBC_BEGIN_DECLS
#		define __LIBC_END_DECLS
#	endif
#endif
