#pragma once

#include "memory/VirtualMemoryManager.hpp"
#include "filesystem/definitions.hpp"
#include <processes/Process.hpp>

namespace Kernel::ELF
{
	// Basic loader to load the dynamic loader which loads the actual program
	Process load(File *file);
}