#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	const char *str = "Hello from this dynamically linked executable!\n";
	write(1, str, strlen(str));
}