#include <dlfcn.h>

int dlclose(void *)
{
	return 0;
}

char *dlerror(void)
{
	return nullptr;
}

void *dlopen(const char *, int)
{
	return nullptr;
}

void *dlsym(void *__restrict, const char *__restrict)
{
	return nullptr;
}
