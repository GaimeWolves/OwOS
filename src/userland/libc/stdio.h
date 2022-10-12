#pragma once

#include <bits/FILE.h>
#include <stdarg.h>
#include <sys/types.h>

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2

#define EOF (-1)

#ifdef __cplusplus
extern "C"
{
#endif

	extern FILE *stdin;
	extern FILE *stdout;
	extern FILE *stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

	int fclose(FILE *);
	int fflush(FILE *);
	FILE *fopen(const char *__restrict, const char *__restrict);
	int fprintf(FILE *__restrict, const char *__restrict, ...) __attribute__ ((format (printf, 2, 3)));
	size_t fread(void *__restrict, size_t, size_t, FILE *__restrict);
	int fseek(FILE *, long, int);
	long ftell(FILE *);
	size_t fwrite(const void *__restrict, size_t, size_t, FILE *__restrict);
	void setbuf(FILE *__restrict, char *__restrict);
	int sprintf(char *__restrict, const char *__restrict, ...);
	int vfprintf(FILE *__restrict, const char *__restrict, va_list);

#ifdef __cplusplus
}
#endif