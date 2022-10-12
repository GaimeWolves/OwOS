#include <test.h>

#include <sys/arch/i386/syscall.h>

void test(const char *str)
{
	syscall(SC_TEST, str);
}