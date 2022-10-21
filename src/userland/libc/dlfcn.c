#include <dlfcn.h>

int dlclose(void *)
{
	return 0;
}

char *dlerror(void)
{
	return 0;
}

void *dlopen(const char *, int)
{
	return 0;
}

void *dlsym(void *__restrict, const char *__restrict)
{
	return 0;
}
