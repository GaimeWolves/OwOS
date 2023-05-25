#pragma once

#include <bits/guards.h>

#include <sys/types.h>

__LIBC_HEADER_BEGIN

struct tm
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

struct timespec
{
	time_t tv_sec;
	long tv_nsec;
};

time_t time(time_t *tloc);

__LIBC_HEADER_END