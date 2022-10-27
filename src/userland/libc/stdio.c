#include <stdio.h>

#include <sys/internals.h>
#include <fcntl.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

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
	return close(stream->fd);
}

int fileno(FILE *stream)
{
	return stream->fd;
}

FILE *fopen(const char *__restrict pathname, const char *__restrict mode)
{
	(void)mode;

	int fd = open(pathname, 0, 0);

	FILE *file = (FILE *)malloc(sizeof(FILE));
	file->fd = fd;

	return file;
}

int fprintf(FILE *__restrict file, const char *__restrict fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int written = vfprintf(file, fmt, ap);
	va_end(ap);
	return written;
}

int fputc(int c, FILE *file)
{
	unsigned char ch = c;
	ssize_t ret = write(file->fd, &ch, 1);

	if (ret < 0)
		return EOF;

	return ch;
}

int printf(const char *__restrict fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int written = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return written;
}

int putchar(int c)
{
	return fputc(c, stdout);
}

int puts(const char *s)
{
	ssize_t ret = write(STDOUT_FILENO, s, strlen(s));

	if (ret < 0)
		return EOF;

	unsigned char ch = '\n';
	ret = write(STDOUT_FILENO, &ch, 1);

	if (ret < 0)
		return EOF;

	return 0;
}

int vfprintf(FILE *__restrict file, const char *__restrict fmt, va_list ap)
{
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
