#pragma once

#include <bits/guards.h>

#define AT_NULL 0
#define AT_IGNORE 1
#define AT_EXECFD 2
#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5
#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_FLAGS 8
#define AT_ENTRY 9
#define AT_LIBPATH 10
#define AT_FPHW 11
#define AT_INTP_DEVICE 12
#define AT_INTP_INODE 13
#define AT_EGID 14
#define AT_PLATFORM 15
#define AT_HWCAP 16
#define AT_CLKTCK 17
#define AT_SECURE 23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM 25
#define AT_HWCAP2 26
#define AT_EXECFN 31
#define AT_EXECBASE 32 // a_ptr holds base address of loaded executable (non-standard type)

__LIBC_HEADER_BEGIN

// https://raw.githubusercontent.com/wiki/hjl-tools/x86-psABI/intel386-psABI-draft.pdf#subsection.2.3.3
typedef struct
{
	int a_type;
	union {
		long a_val;
		void *a_ptr;
		void (*a_fcn)();
	} a_un;
} auxv_t;

__LIBC_HEADER_END