#pragma once

#include <bits/guards.h>

__LIBC_HEADER_BEGIN

void __libc_init(int argc, char **argv);
void __stdio_init();
void __malloc_init();

__LIBC_HEADER_END