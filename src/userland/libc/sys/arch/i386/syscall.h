#pragma once

#include <stdint.h>

#define __ENUM_SYSCALL(S) \
	S(test)

enum Syscall
{
#define __ENUM_FN(syscall) SC_##syscall,
	__ENUM_SYSCALL(__ENUM_FN)
#undef __ENUM_FN
};

#ifndef __OWOS_KERNEL
#	undef __ENUM_SYSCALL
#endif

inline uintptr_t syscall(Syscall syscall)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	    : "=a" (ret)
	    : "a" (syscall)
	    : "memory");

	return ret;
}

template <typename T1>
inline uintptr_t syscall(Syscall syscall, T1 arg1)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	             : "=a" (ret)
	             : "a" (syscall), "b" ((uintptr_t)arg1)
	             : "memory");

	return ret;
}

template <typename T1, typename T2>
inline uintptr_t syscall(Syscall syscall, T1 arg1, T2 arg2)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	             : "=a" (ret)
	             : "a" (syscall), "b" ((uintptr_t)arg1), "c" ((uintptr_t)arg2)
	             : "memory");

	return ret;
}

template <typename T1, typename T2, typename T3>
inline uintptr_t syscall(Syscall syscall, T1 arg1, T2 arg2, T3 arg3)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	             : "=a" (ret)
	             : "a" (syscall), "b" ((uintptr_t)arg1), "c" ((uintptr_t)arg2), "d" ((uintptr_t)arg3)
	             : "memory");

	return ret;
}

template <typename T1, typename T2, typename T3, typename T4>
inline uintptr_t syscall(Syscall syscall, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	             : "=a" (ret)
	             : "a" (syscall), "b" ((uintptr_t)arg1), "c" ((uintptr_t)arg2), "d" ((uintptr_t)arg3), "D" ((uintptr_t)arg4)
	             : "memory");

	return ret;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline uintptr_t syscall(Syscall syscall, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
{
	uintptr_t ret;

	asm volatile("int $0x80"
	             : "=a" (ret)
	             : "a" (syscall), "b" ((uintptr_t)arg1), "c" ((uintptr_t)arg2), "d" ((uintptr_t)arg3), "D" ((uintptr_t)arg4), "S" ((uintptr_t)arg5)
	             : "memory");

	return ret;
}
