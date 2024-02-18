#pragma once

#ifdef ARCH_i686
#	include <arch/i686/process.hpp>
#else
#	error No arch-specific Processor.hpp included
#endif
