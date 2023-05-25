#pragma once

#include <bits/guards.h>
#include <bits/FILE.h>
#include <stdarg.h>
#include <sys/types.h>

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

#define BUFSIZ 1

#define EOF (-1)

__LIBC_HEADER_BEGIN

	extern FILE *stdin;
	extern FILE *stdout;
	extern FILE *stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

	int fclose(FILE *);

    int fflush(FILE *stream);
    int fileno(FILE *);

    int feof(FILE *stream);
    int ferror(FILE *stream);
    void clearerr(FILE *stream);

	FILE *fopen(const char *__restrict, const char *__restrict);
    FILE *fdopen(int fildes, const char *mode);
    FILE *freopen(const char *__restrict pathname, const char *__restrict mode, FILE *__restrict stream);
    int fputc(int, FILE *);
    int fputs(const char *__restrict s, FILE *__restrict stream);
    int fgetc(FILE *stream);
    char *fgets(char *__restrict s, int n, FILE *__restrict stream);
    size_t fread(void *__restrict ptr, size_t size, size_t nitems, FILE *__restrict stream);
    int fseek(FILE *stream, long offset, int whence);
    void rewind(FILE *stream);
    long ftell(FILE *stream);
    size_t fwrite(const void *__restrict ptr, size_t size, size_t nitems, FILE *__restrict stream);

    int putchar(int);

    int putc(int c, FILE *stream);
    int puts(const char *);

    void perror(const char *s);

	void setbuf(FILE *__restrict, char *__restrict);

    int printf(const char *__restrict, ...);
    int fprintf(FILE *__restrict, const char *__restrict, ...) __attribute__ ((format (printf, 2, 3)));
    int sprintf(char *__restrict s, const char *__restrict format, ...);
    int snprintf(char *__restrict s, size_t n, const char *__restrict format, ...);

	int vfprintf(FILE *__restrict, const char *__restrict, va_list);
    int vprintf(const char *__restrict, va_list);
    int vsprintf(char *__restrict s, const char *__restrict format, va_list ap);
    int vsnprintf(char *__restrict s, size_t n, const char *__restrict format, va_list ap);

    int sscanf(const char *__restrict s, const char *__restrict format, ...);

    char *tmpnam(char *s);

__LIBC_HEADER_END