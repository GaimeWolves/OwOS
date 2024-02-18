#pragma once

#include "../../userland/libc/signal.h"
#include "../../userland/libc/sys/arch/i386/syscall.h"
#include "../../userland/libc/termios.h"

#include <sys/stat.h>

#include <arch/process.hpp>
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
	[[noreturn]] uintptr_t syscall$exit(int exit_code);
	uintptr_t syscall$exec(const char *path, const char *argv[], const char *envp[]);
	uintptr_t syscall$fork();
	uintptr_t syscall$waitpid(pid_t pid, int *stat_loc, int options);
	uintptr_t syscall$getcwd(char *buf, size_t size);
	uintptr_t syscall$chdir(char *path);
	uintptr_t syscall$getdents(int fd, void *buffer, size_t count);
	uintptr_t syscall$sigaction(int signal, const struct sigaction *act, struct sigaction *oact);
	uintptr_t syscall$sigreturn(thread_registers_t *original_regs, CPU::interrupt_frame_t *frame);
}
