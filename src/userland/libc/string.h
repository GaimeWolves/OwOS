#pragma once

#include <bits/guards.h>
#include <stddef.h>

__LIBC_BEGIN_DECLS

void  *memchr(const void *s, int c, size_t n);
int    memcmp(const void *s1, const void *s2, size_t n);
void  *memcpy(void *__restrict s1, const void *__restrict s2, size_t n);
void  *memmove(void *s1, const void *s2, size_t n);
void  *memset(void *s, int c, size_t n);
char  *strcat(char *__restrict s1, const char *__restrict s2);
char  *strchr(const char *s, int c);
int    strcmp(const char *s1, const char *s2);
char  *strcpy(char *__restrict s1, const char *__restrict s2);
char  *strdup(const char *s);
char  *strerror(int errnum);
size_t strlen(const char *s);
char  *strncat(char *__restrict s1, const char *__restrict s2, size_t n);
int    strncmp(const char *s1, const char *s2, size_t n);
char  *strncpy(char *__restrict s1, const char *__restrict s2, size_t n);
char  *strpbrk(const char *s1, const char *s2);
char  *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
char  *strstr(const char *s1, const char *s2);
char  *strtok(char *__restrict s, const char *__restrict sep);

__LIBC_END_DECLS
