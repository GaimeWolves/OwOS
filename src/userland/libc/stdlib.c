#include <stdlib.h>

void abort(void)
{
	for (;;)
		;
}

void exit(int)
{
	for (;;)
		;
}

void free(void *)
{

}

void *malloc(size_t)
{
	return 0;
}
