#pragma once

#include "../../userland/libc/sys/arch/i386/syscall.h"
#include "../../userland/libc/termios.h"

#include <sys/stat.h>

#include <logging/logger.hpp>
#include <libk/kcstdio.hpp>

namespace Kernel
{
	uintptr_t syscall$mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
	uintptr_t syscall$munmap(void *addr, size_t len);
	uintptr_t syscall$open(const char *path, int oflag, unsigned mode);
	uintptr_t syscall$close(int fd);
	uintptr_t syscall$read(int fd, void *buf, size_t count);
	uintptr_t syscall$write(int fd, void *buf, size_t count);
	uintptr_t syscall$stat(int type, int cwd, const char *path, int fd, struct stat *buf, int flags);
	uintptr_t syscall$ioctl(int fd, uint32_t request, uintptr_t arg);
}