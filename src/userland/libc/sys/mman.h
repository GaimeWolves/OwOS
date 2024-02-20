#pragma once

#include <bits/guards.h>
#include <stddef.h>
#include <sys/types.h>

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

#define MAP_SHARED    0x01
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x04
#define MAP_ANONYMOUS 0x08
#define MAP_ANON      MAP_ANONYMOUS

#define MAP_FAILED (-1)

__LIBC_BEGIN_DECLS

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int   munmap(void *addr, size_t len);

__LIBC_END_DECLS
