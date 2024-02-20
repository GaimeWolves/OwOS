#pragma once

#define always_inline inline __attribute__((always_inline))

#define __packed __attribute__((packed))

#ifndef __noreturn
#	define __noreturn __attribute__((noreturn))
#endif

#define __naked       __attribute__((naked))
#define __unused      __attribute__((unused))
#define __used        __attribute__((used))
#define __constructor __attribute__((constructor))

#define __section(x)      __attribute__((section(x)))
#define __packed_align(x) __attribute__((packed, aligned(x)))

#define KiB 1024
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)
#define TiB (1024 * GiB)
