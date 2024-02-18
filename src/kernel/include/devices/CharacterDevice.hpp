#pragma once

#include <devices/Device.hpp>

namespace Kernel
{
	class CharacterDevice : public Device
	{
	public:
		CharacterDevice(size_t major, size_t minor)
			: Device(major, minor)
		{
		}
	};
}
