#pragma once

#define LC_CTYPE 1

#ifdef __cplusplus
extern "C"
{
#endif

	char *setlocale(int category, const char *locale);

#ifdef __cplusplus
}
#endif
