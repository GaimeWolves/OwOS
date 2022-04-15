#pragma once

#include <storage/StorageDevice.hpp>
#include <storage/ata/AHCIController.hpp>

namespace Kernel
{
	class AHCIManager
	{
	public:
		static AHCIManager &instance()
		{
			static AHCIManager *instance{nullptr};

			if (!instance)
				instance = new AHCIManager();

			return *instance;
		}

		void initialize();

		[[nodiscard]] const LibK::vector<StorageDevice> &devices() const { return m_connected_devices; }

	private:
		AHCIManager() = default;
		~AHCIManager() = default;

		LibK::vector<AHCIController> m_controllers{};
		LibK::vector<StorageDevice> m_connected_devices{};
	};
}