#pragma once

#define always_inline inline __attribute__((always_inline))

#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))
#define __naked __attribute__((naked))
#define __unused __attribute__((unused))
#define __constructor __attribute__((constructor))

#define __section(x) __attribute__((section(x)))
#define __packed_align(x) __attribute__((packed, aligned(x)))