#include <storage/ata/AHCIManager.hpp>

#include <pci/pci.hpp>

namespace Kernel
{
	void AHCIManager::initialize()
	{
		PCI::HostBridge::instance().enumerate([&](PCI::Function &function) {
			// TODO: Check vendor and device id
			if (function.get_class() == 0x01 && function.get_subclass() == 0x06)
				m_controllers.push_back(AHCIController(function));
		});
	}
}