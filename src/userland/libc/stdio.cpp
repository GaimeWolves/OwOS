#include <stdio.h>

#undef stdin
#undef stdout
#undef stderr

struct FILE
{
	unsigned int fd;
};

static FILE s_standard_streams[3] = {
    FILE { .fd = 0, },
    FILE { .fd = 1, },
    FILE { .fd = 2, },
};

FILE *stdin = &s_standard_streams[0];
FILE *stdout = &s_standard_streams[1];
FILE *stderr = &s_standard_streams[2];

int fclose(FILE *)
{
	return 0;
}

int fflush(FILE *)
{
	return 0;
}

FILE *fopen(const char *__restrict, const char *__restrict)
{
	return nullptr;
}

int fprintf(FILE *__restrict, const char *__restrict, ...)
{
	return 0;
}

size_t fread(void *__restrict, size_t, size_t, FILE *__restrict)
{
	return 0;
}

int fseek(FILE *, long, int)
{
	return 0;
}

long ftell(FILE *)
{
	return 0;
}

size_t fwrite(const void *__restrict, size_t, size_t, FILE *__restrict)
{
	return 0;
}

void setbuf(FILE *__restrict, char *__restrict)
{

}

int sprintf(char *__restrict, const char *__restrict, ...)
{
	return 0;
}

int vfprintf(FILE *__restrict, const char *__restrict, va_list)
{
	return 0;
}
