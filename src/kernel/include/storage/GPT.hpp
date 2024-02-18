#pragma once

#include <storage/StorageDevice.hpp>

namespace Kernel::GPT
{
	bool try_parse(StorageDevice &device);
}
