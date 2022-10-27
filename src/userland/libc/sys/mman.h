#pragma once

#include <bits/guards.h>
#include <stddef.h>
#include <sys/types.h>

#define PROT_NONE 0x00
#define PROT_READ 0x01
#define PROT_WRITE 0x02
#define PROT_EXEC 0x04

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x04
#define MAP_ANONYMOUS 0x08
#define MAP_ANON MAP_ANONYMOUS

#define MAP_FAILED (-1)

__LIBC_HEADER_BEGIN

void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);

__LIBC_HEADER_END