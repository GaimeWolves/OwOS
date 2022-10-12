#include <test.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	test("Hello from an ELF position independent executable");

	for (;;)
		;
}