#pragma once

#include <bits/attributes.h>
#include <bits/guards.h>
#include <stddef.h>

__LIBC_BEGIN_DECLS

__noreturn void    abort(void);
int                abs(int i);
int                atexit(void (*func)(void));
int                atoi(const char *str);
void              *calloc(size_t nelem, size_t elsize);
__noreturn void    exit(int status);
void               free(void *ptr);
char              *getenv(const char *name);
void              *malloc(size_t size);
int                putenv(char *string);
void               qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *));
void              *realloc(void *ptr, size_t size);
int                setenv(const char *envname, const char *envval, int overwrite);
long               strtol(const char *__restrict nptr, char **__restrict endptr, int base);
long long          strtoll(const char *__restrict nptr, char **__restrict endptr, int base);
unsigned long      strtoul(const char *__restrict nptr, char **__restrict endptr, int base);
unsigned long long strtoull(const char *__restrict nptr, char **__restrict endptr, int base);
int                system(const char *command);

__LIBC_END_DECLS
