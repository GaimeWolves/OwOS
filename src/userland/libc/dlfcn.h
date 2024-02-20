#pragma once

#include <bits/guards.h>

__LIBC_BEGIN_DECLS

int   dlclose(void *handle);
char *dlerror(void);
void *dlopen(const char *file, int mode);
void *dlsym(void *__restrict handle, const char *__restrict name);

__LIBC_END_DECLS
