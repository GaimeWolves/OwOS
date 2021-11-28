#pragma once

#ifdef ARCH_i686
#	include <arch/i686/spinlock.hpp>
#else
#	error No arch-specific spinlock.hpp included
#endif