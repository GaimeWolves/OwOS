#include <stdio.h>

#include <__debug.h>

#include <sys/internals.h>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "__printf.h"

#undef stdin
#undef stdout
#undef stderr

struct FILE
{
	int fd;
};

static FILE s_standard_streams[3];

FILE *stdin;
FILE *stdout;
FILE *stderr;

void __stdio_init()
{
	s_standard_streams[0].fd = 0;
	s_standard_streams[1].fd = 1;
	s_standard_streams[2].fd = 2;
	stdin = &s_standard_streams[0];
	stdout = &s_standard_streams[1];
	stderr = &s_standard_streams[2];
}

int fclose(FILE *stream)
{
	TRACE("fclose(%p)\r\n", stream);

	return close(stream->fd);
}

int fflush(FILE *stream)
{
	TRACE("fflush(%p)\r\n", stream);
	(void)stream;
	puts("fflush() not fully implemented, as no buffering is done at the moment");
	return 0;
}

int fileno(FILE *stream)
{
	TRACE("fileno(%p)\r\n", stream);

	return stream->fd;
}

int feof(FILE *stream)
{
	TRACE("feof(%p)\r\n", stream);

	(void)stream;
	puts("feof() not implemented");
	abort();
}

int ferror(FILE *stream)
{
	TRACE("ferror(%p)\r\n", stream);

	(void)stream;
	puts("ferror() not implemented");
	abort();
}

void clearerr(FILE *stream)
{
	TRACE("clearerr(%p)\r\n", stream);

	(void)stream;
	puts("clearerr() not implemented");
	abort();
}

FILE *fopen(const char *__restrict pathname, const char *__restrict mode)
{
	TRACE("fopen(%s, %s)\r\n", pathname, mode);

	(void)mode;

	int fd = open(pathname, 0, 0);

	FILE *file = (FILE *)malloc(sizeof(FILE));
	file->fd = fd;

	return file;
}

FILE *fdopen(int fildes, const char *mode)
{
	TRACE("fdopen(%d, %s)\r\n", fildes, mode);

	(void)mode;

	FILE *file = (FILE *)malloc(sizeof(FILE));
	file->fd = fildes;

	return file;
}

FILE *freopen(const char *restrict pathname, const char *restrict mode, FILE *restrict stream)
{
	TRACE("freopen(%s, %s, %p)\r\n", pathname, mode, stream);

	(void)pathname;
	(void)mode;
	(void)stream;
	puts("freopen() not implemented");
	abort();
}

int fputc(int c, FILE *file)
{
	// TRACE("fputc(%c, %p)\r\n", c, file);

	unsigned char ch = c;
	ssize_t ret = write(file->fd, &ch, 1);

	if (ret < 0)
		return EOF;

	return ch;
}

int fputs(const char *restrict s, FILE *restrict stream)
{
	TRACE("fputs(%s, %p)\r\n", s, stream);

	return write(fileno(stream), s, strlen(s) + 1);
}

int fgetc(FILE *stream)
{
	TRACE("fgetc(%p)\r\n", stream);

	int ch = 0;
	ssize_t ret = read(stream->fd, &ch, 1);

	if (ret < 0)
		return EOF;

	return ch;
}

char *fgets(char *restrict s, int n, FILE *restrict stream)
{
	TRACE("fgets(%s, %d, %p)\r\n", s, n, stream);

	(void)s;
	(void)n;
	(void)stream;
	puts("fgets() not implemented");
	abort();
}

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/fread.html
size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
{
	TRACE("fread(%p, %lu, %lu, %p)\r\n", ptr, size, nitems, stream);

	if (!ptr || !stream)
		return 0;

	// NOTE: The spec says:
	// For each object, size calls shall be made to the fgetc() function and the results stored,
	// in the order read, in an array of unsigned char exactly overlaying the object.
	//
	// But a single call to read is equivalent for now

	return read(stream->fd, ptr, nitems * size);
}

int fseek(FILE *stream, long offset, int whence)
{
	TRACE("fseek(%p, %ld, %d)\r\n", stream, offset, whence);

	(void)stream;
	(void)offset;
	(void)whence;
	puts("fseek() not implemented");
	abort();
}

void rewind(FILE *stream)
{
	TRACE("rewind(%p)\r\n", stream);

	(void)stream;
	puts("rewind() not implemented");
	abort();
}

long ftell(FILE *stream)
{
	TRACE("ftell(%p)\r\n", stream);

	(void)stream;
	puts("ftell() not implemented");
	abort();
}

size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
{
	TRACE("fwrite(%p, %lu, %lu, %p)\r\n", ptr, size, nitems, stream);

	size_t num = 0;
	const char *data = ptr;
	while (nitems--) {
		size_t count = size;
		while (count--) {
			fputc(*data++, stream);
		}
		num++;
	}

	return num;
}

int putchar(int c)
{
	return fputc(c, stdout);
}

int putc(int c, FILE *stream)
{
	(void)c;
	(void)stream;
	puts("putc() not implemented");
	abort();
}

int puts(const char *s)
{
	ssize_t ret = write(STDOUT_FILENO, s, strlen(s));

	if (ret < 0)
		return EOF;

	unsigned char ch = '\r';
	ret = write(STDOUT_FILENO, &ch, 1);

	if (ret < 0)
		return EOF;

	ch = '\n';
	ret = write(STDOUT_FILENO, &ch, 1);

	if (ret < 0)
		return EOF;

	return 0;
}

void perror(const char *s)
{
	if (s && *s)
		fprintf(stderr, "%s: %s\r\n", s, strerror(errno));
	else
		fprintf(stderr, "%s\r\n", strerror(errno));
}

int printf(const char *__restrict fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int written = vfprintf(stdout, fmt, ap);

	va_end(ap);

	return written;
}

int fprintf(FILE *__restrict file, const char *__restrict fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int written = vfprintf(file, fmt, ap);

	va_end(ap);

	return written;
}

int sprintf(char *restrict s, const char *restrict format, ...)
{
	TRACE("sprintf(%p, %s)\r\n", s, format);

	va_list ap;
	va_start(ap, format);

	int written = vsnprintf(s, INT_MAX, format, ap);

	va_end(ap);

	return written;
}

int snprintf(char *restrict s, size_t n, const char *restrict format, ...)
{
	va_list ap;
	va_start(ap, format);

	int written = vsnprintf(s, n, format, ap);

	va_end(ap);

	return written;
}

int vfprintf(FILE *__restrict file, const char *__restrict fmt, va_list ap)
{
	TRACE("vfprintf(%p, %s, %p)\r\n", file, fmt, ap);

	printf_conv_t conversion = { 0 };

	conversion.ap = ap;
	conversion.file = file;
	conversion.bufsz = INT_MAX;
	conversion.format = fmt;

	__printf_impl(&conversion);

	return conversion.written;
}

int vprintf(const char *__restrict format, va_list ap)
{
	return vfprintf(stdout, format, ap);
}

int vsprintf(char *restrict s, const char *restrict format, va_list ap)
{
	return vsnprintf(s, INT_MAX, format, ap);
}

int vsnprintf(char *restrict s, size_t n, const char *restrict format, va_list ap)
{
	TRACE("vsnprintf(%p, %lu, %s, %p)\r\n", s, n, format, ap);

	printf_conv_t conversion = { 0 };

	conversion.ap = ap;
	conversion.buffer = s;
	conversion.bufsz = n;
	conversion.format = format;

	__printf_impl(&conversion);

	return conversion.written;
}

int sscanf(const char *restrict s, const char *restrict format, ...)
{
	TRACE("sscanf(%s, %s)\r\n", s, format);

	(void)s;
	(void)format;
	puts("sscanf() not implemented");
	abort();
}

char *tmpnam(char *s)
{
	TRACE("tmpnam(%s)\r\n", s);

	(void)s;
	puts("tmpnam() not implemented");
	abort();
}
