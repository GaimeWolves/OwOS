#include <test.h>

#include <sys/arch/i386/syscall.h>

void test(const char *str)
{
	syscall(__SC_test, str);
}