__attribute__((noreturn)) int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	__asm__ __volatile__("int $0x80" : : "b" ("Hello from an ELF position independent shared object"), "a" (0) : "memory");

	for (;;)
		;
}