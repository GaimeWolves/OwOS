#pragma once

#include <stdint.h>

#include <libk/kcstring.hpp>

namespace Kernel::LibK
{
	class GUID
	{
	public:
		static constexpr size_t GUID_LENGTH = 16;

		GUID()
		{
			memset(m_guid, 0, GUID_LENGTH);
		}

		constexpr explicit GUID(const uint8_t *data)
		{
			for (size_t i = 0; i < GUID_LENGTH; i++)
				m_guid[i] = data[i];
		}

		bool operator==(const GUID &other) const
		{
			return memcmp(m_guid, other.m_guid, GUID_LENGTH) == 0;
		}

	private:
		uint8_t m_guid[GUID_LENGTH]{};
	};
}