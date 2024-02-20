#pragma once

#include <bits/FILE.h>
#include <bits/guards.h>
#include <stdarg.h>
#include <sys/types.h>

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

#define BUFSIZ 1

#define EOF (-1)

#define stdin  stdin
#define stdout stdout
#define stderr stderr

__LIBC_BEGIN_DECLS

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

void   clearerr(FILE *stream);
int    fclose(FILE *stream);
FILE  *fdopen(int fildes, const char *mode);
int    feof(FILE *stream);
int    ferror(FILE *stream);
int    fflush(FILE *stream);
int    fgetc(FILE *stream);
char  *fgets(char *__restrict s, int n, FILE *__restrict stream);
int    fileno(FILE *stream);
FILE  *fopen(const char *__restrict base, const char *__restrict mode);
int    fprintf(FILE *__restrict stream, const char *__restrict format, ...);
int    fputc(int c, FILE *stream);
int    fputs(const char *__restrict s, FILE *__restrict stream);
size_t fread(void *__restrict ptr, size_t size, size_t nitems, FILE *__restrict stream);
FILE  *freopen(const char *__restrict pathname, const char *__restrict mode, FILE *__restrict stream);
int    fseek(FILE *stream, long offset, int whence);
long   ftell(FILE *stream);
size_t fwrite(const void *__restrict ptr, size_t size, size_t nitems, FILE *__restrict stream);
void   perror(const char *s);
int    printf(const char *__restrict format, ...);
int    putc(int c, FILE *stream);
int    putchar(int c);
int    puts(const char *s);
void   rewind(FILE *stream);
void   setbuf(FILE *__restrict stream, char *__restrict buf);
int    snprintf(char *__restrict s, size_t n, const char *__restrict format, ...);
int    sprintf(char *__restrict s, const char *__restrict format, ...);
int    sscanf(const char *__restrict s, const char *__restrict format, ...);
char  *tmpnam(char *s);
int    vfprintf(FILE *__restrict stream, const char *__restrict format, va_list ap);
int    vprintf(const char *__restrict format, va_list ap);
int    vsnprintf(char *__restrict s, size_t n, const char *__restrict format, va_list ap);
int    vsprintf(char *__restrict s, const char *__restrict format, va_list ap);

__LIBC_END_DECLS
