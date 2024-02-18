#include <storage/ata/AHCIManager.hpp>

#include <pci/pci.hpp>

namespace Kernel
{
	void AHCIManager::initialize()
	{
		PCI::HostBridge::instance().enumerate([&](PCI::Function &function) {
			// TODO: Check vendor and device id
			if (function.get_class() == 0x01 && function.get_subclass() == 0x06)
				m_controllers.emplace_back(function);
		});

		for (auto &controller : m_controllers)
		{
			controller.initialize();
			for (size_t i = 0; i < AHCI::NUM_PORTS; i++)
			{
				if (controller.ports()[i].attached())
				{
					m_connected_devices.emplace_back(&controller.ports()[i]);
					m_connected_devices.back().initialize();
				}
			}
		}
	}
}
