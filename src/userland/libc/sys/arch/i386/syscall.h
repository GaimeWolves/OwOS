#pragma once

#include <sys/types.h>
#include <errno.h>
#include <stdint.h>

#define __ENUM_SYSCALL(S) \
	S(test)               \
	S(mmap)                  \
	S(read)                  \
	S(write)

typedef enum Syscall
{
#define __ENUM_FN(syscall) __SC_##syscall,
	__ENUM_SYSCALL(__ENUM_FN)
#undef __ENUM_FN
} Syscall;

#ifndef __OWOS_KERNEL
#	undef __ENUM_SYSCALL
#endif

// https://github.com/managarm/mlibc/blob/master/sysdeps/linux/include/bits/syscall.h
typedef uintptr_t __sc_word_t;

#define __scc(x) ((__sc_word_t)(x))
#define __syscall0(n) __do_syscall0(n)
#define __syscall1(n,a) __do_syscall1(n,__scc(a))
#define __syscall2(n,a,b) __do_syscall2(n,__scc(a),__scc(b))
#define __syscall3(n,a,b,c) __do_syscall3(n,__scc(a),__scc(b),__scc(c))
#define __syscall4(n,a,b,c,d) __do_syscall4(n,__scc(a),__scc(b),__scc(c),__scc(d))
#define __syscall5(n,a,b,c,d,e) __do_syscall5(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __SYSCALL_NARGS_X(a,b,c,d,e,f,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define __SYSCALL_DISP(b,...) __SYSCALL_CONCAT(b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define __syscall(...) __SYSCALL_DISP(__syscall,__VA_ARGS__)
#define syscall(...) __do_syscall_ret(__syscall(__VA_ARGS__))

uintptr_t __do_syscall_ret(uintptr_t ret);
uintptr_t __do_syscall0(Syscall syscall);
uintptr_t __do_syscall1(Syscall syscall, __sc_word_t arg1);
uintptr_t __do_syscall2(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2);
uintptr_t __do_syscall3(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3);
uintptr_t __do_syscall4(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3, __sc_word_t arg4);
uintptr_t __do_syscall5(Syscall syscall, __sc_word_t arg1, __sc_word_t arg2, __sc_word_t arg3, __sc_word_t arg4, __sc_word_t arg5);

typedef struct SC_mmap_params_t
{
	void *addr;
	size_t len;
	int prot;
	int flags;
	int fildes;
	off_t off;
} SC_mmap_params_t;
