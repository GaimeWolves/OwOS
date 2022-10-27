#include <sys/mman.h>

#include <sys/arch/i386/syscall.h>

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	SC_mmap_params_t params = {
		.addr = addr,
	    .len = len,
	    .prot = prot,
	    .flags = flags,
	    .fildes = fildes,
	    .off = off,
	};

	uintptr_t ret = syscall(__SC_mmap, &params);

	if (ret == -1UL)
		return (void *)(MAP_FAILED);

	return (void *)(ret);
}

int munmap(void *addr, size_t len)
{
	return syscall(__SC_munmap, addr, len);
}