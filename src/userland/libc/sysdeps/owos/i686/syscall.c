#include <sys/syscall.h>

// See https://github.com/managarm/mlibc/blob/a6b714a834d1da97d8231cb4bc8ae4a3d7885dd9/sysdeps/linux/generic/sysdeps.cpp
uintptr_t __do_syscall_ret(uintptr_t ret)
{
	if (ret > -4096UL)
	{
		errno = -ret;
		return -1;
	}

	return ret;
}

uintptr_t __do_syscall0(Syscall syscall)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	             : "=a"(ret)
	             : "a"(syscall)
	             : "memory");

	return ret;
}

uintptr_t __do_syscall1(Syscall syscall, __sc_word_t arg1)
{
	uintptr_t ret;

	asm volatile(
	    "int $0x80"
	    : "=a"(ret)
	    : "a"(syscall), "b"(arg1)
	    : "memory");

	return ret;
}

uintptr_t __do_syscall2(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2)
{
	uintptr_t ret;

	asm volatile(
	    "int $0x80"
	    : "=a"(ret)
	    : "a"(syscall), "b"(arg1), "c"(arg2)
	    : "memory");

	return ret;
}

uintptr_t __do_syscall3(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3)
{
	uintptr_t ret;

	asm volatile(
	    "int $0x80"
	    : "=a"(ret)
	    : "a"(syscall), "b"(arg1), "c"(arg2), "d"(arg3)
	    : "memory");

	return ret;
}

uintptr_t __do_syscall4(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3, __sc_word_t arg4)
{
	uintptr_t ret;

	asm volatile(
	    "int $0x80"
	    : "=a"(ret)
	    : "a"(syscall), "b"(arg1), "c"(arg2), "d"(arg3), "D"(arg4)
	    : "memory");

	return ret;
}

uintptr_t __do_syscall5(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3, __sc_word_t arg4, __sc_word_t arg5)
{
	uintptr_t ret;

	asm volatile(
	    "int $0x80"
	    : "=a"(ret)
	    : "a"(syscall), "b"(arg1), "c"(arg2), "d"(arg3), "D"(arg4), "S"(arg5)
	    : "memory");

	return ret;
}

uintptr_t __do_syscall6(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3, __sc_word_t arg4, __sc_word_t arg5, __sc_word_t arg6)
{
	uintptr_t ret;

	asm volatile(
	    "push %%ebp\n"
	    "push %[ebp]\n"
	    "pop %%ebp\n"
	    "int $0x80\n"
	    "pop %%ebp"
	    : "=a"(ret)
	    : "a"(syscall), "b"(arg1), "c"(arg2), "d"(arg3), "D"(arg4), "S"(arg5), [ebp] "m"(arg6)
	    : "memory");

	return ret;
}
