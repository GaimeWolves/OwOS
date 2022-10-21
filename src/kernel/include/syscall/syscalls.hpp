#pragma once

#include "../../userland/libc/sys/arch/i386/syscall.h"

#include <logging/logger.hpp>
#include <libk/kcstdio.hpp>

namespace Kernel
{
	uint32_t syscall$test(const char *str);
	uintptr_t syscall$mmap(SC_mmap_params_t *params);
	uintptr_t syscall$read(int fd, void *buf, size_t count);
	uintptr_t syscall$write(int fd, void *buf, size_t count);
}