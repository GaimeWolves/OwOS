#include <time.h>

#include <__debug.h>

#include <stdio.h>

time_t time(time_t *tloc)
{
	TRACE("time(%p)\r\n", tloc);
	// puts("time() not actually working! Set to 2023/01/01 00:00:00!");

	if (tloc)
		*tloc = 1672531200;

	return 1672531200;
}
