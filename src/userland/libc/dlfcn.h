#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	int dlclose(void *);
	char *dlerror(void);
	void *dlopen(const char *, int);
	void *dlsym(void *__restrict, const char *__restrict);

#ifdef __cplusplus
}
#endif
