#include <panic.hpp>

#include <stdint.h>

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0x6e8bd94b
#else
#define STACK_CHK_GUARD 0x12796f1e09b49319
#endif

extern uintptr_t __stack_chk_guard;
__used uintptr_t __stack_chk_guard = (uintptr_t)STACK_CHK_GUARD;

extern "C" __noreturn __used void __stack_chk_fail(void)
{
	Kernel::panic("Stack smashed");
}

extern "C" __noreturn __used void __stack_chk_fail_local(void)
{
	__stack_chk_fail();
}
