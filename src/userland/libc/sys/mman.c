#include <sys/mman.h>

#include <__debug.h>

#include <sys/arch/i386/syscall.h>

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	TRACE("mmap(%p, %lu, %d, %d, %d, %lu)\r\n", addr, len, prot, flags, fildes, off);

	uintptr_t ret = syscall(__SC_mmap, addr, len, prot, flags, fildes, off);

	if (ret == -1UL)
		return (void *)(MAP_FAILED);

	return (void *)(ret);
}

int munmap(void *addr, size_t len)
{
	TRACE("munmao(%p, %lu)\r\n", addr, len);

	return syscall(__SC_munmap, addr, len);
}
