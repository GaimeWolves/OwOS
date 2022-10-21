#include <sys/mman.h>
#include <test.h>
#include <errno.h>
#include <string.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	void *ptr = mmap((void *)0x100000, 0x1500, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if ((int)ptr == MAP_FAILED)
	{
		// test("mmap FAILED with error:");
		test(strerror(errno));
	}
	else
	{
		test("mmap SUCCESS");
	}

	ptr = mmap((void *)0x100000, 0, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if ((int)ptr == MAP_FAILED)
	{
		// test("mmap FAILED with error:");
		test(strerror(errno));
	}
	else
	{
		test("mmap SUCCESS");
	}

	for (;;)
		;
}