#include <pci/pci.hpp>

#include <arch/io.hpp>

#include <libk/kcstdio.hpp>

namespace Kernel::PCI
{
	static IO::Port config_addr(PCI_CONFIG_ADDR_IO_PORT);
	static IO::Port config_data(PCI_CONFIG_DATA_IO_PORT);

	uint32_t Function::read_32(uint8_t offset)
	{
		config_addr.out(m_address(offset));
		return config_data.in<uint32_t>();
	}

	uint16_t Function::read_16(uint8_t offset)
	{
		config_addr.out(m_address(offset));
		return config_data.offset(offset & 2).in<uint16_t>();
	}

	uint8_t Function::read_8(uint8_t offset)
	{
		config_addr.out(m_address(offset));
		return config_data.offset(offset & 3).in<uint8_t>();
	}

	void Function::write_32(uint8_t offset, uint32_t value)
	{
		config_addr.out(m_address(offset));
		config_data.out<uint32_t>(value);
	}

	void Function::write_16(uint8_t offset, uint16_t value)
	{
		config_addr.out(m_address(offset));
		config_data.offset(offset & 2).out<uint16_t>(value);
	}

	void Function::write_8(uint8_t offset, uint8_t value)
	{
		config_addr.out(m_address(offset));
		config_data.offset(offset & 4).out<uint8_t>(value);
	}

	uint16_t Function::get_device_id()
	{
		return read_16(PCI_REG_DEVICE_ID);
	}

	uint16_t Function::get_vendor_id()
	{
		return read_16(PCI_REG_VENDOR_ID);
	}

	uint16_t Function::get_status()
	{
		return read_16(PCI_REG_STATUS);
	}

	void Function::set_command(uint16_t command)
	{
		write_16(PCI_REG_COMMAND, command);
	}

	uint16_t Function::get_command()
	{
		return read_16(PCI_REG_COMMAND);
	}

	uint8_t Function::get_revision_id()
	{
		return read_8(PCI_REG_REVISION_ID);
	}

	uint8_t Function::get_prog_if()
	{
		return read_8(PCI_REG_PROG_IF);
	}

	uint8_t Function::get_subclass()
	{
		return read_8(PCI_REG_SUBCLASS);
	}

	uint8_t Function::get_class()
	{
		return read_8(PCI_REG_CLASS);
	}

	void Function::set_cache_line_size(uint8_t size)
	{
		write_8(PCI_REG_CACHE_LINE_SIZE, size);
	}

	uint8_t Function::get_latency()
	{
		return read_8(PCI_REG_LATENCY_TIMER);
	}

	uint8_t Function::get_header_type()
	{
		return read_8(PCI_REG_HEADER_TYPE);
	}

	uint8_t Function::get_bist()
	{
		return read_8(PCI_REG_BIST);
	}

	void Function::set_bist(uint8_t value)
	{
		write_8(PCI_REG_BIST, value);
	}

	uint32_t Function::get_bar0()
	{
		return read_32(PCI_REG_BAR0);
	}

	void Function::set_bar0(uint32_t address)
	{
		write_32(PCI_REG_BAR0, address);
	}

	uint32_t Function::get_bar1()
	{
		return read_32(PCI_REG_BAR1);
	}

	void Function::set_bar1(uint32_t address)
	{
		write_32(PCI_REG_BAR1, address);
	}

	uint32_t Function::get_bar2()
	{
		return read_32(PCI_REG_BAR2);
	}

	void Function::set_bar2(uint32_t address)
	{
		write_32(PCI_REG_BAR2, address);
	}

	uint32_t Function::get_bar3()
	{
		return read_32(PCI_REG_BAR3);
	}

	void Function::set_bar3(uint32_t address)
	{
		write_32(PCI_REG_BAR3, address);
	}

	uint32_t Function::get_bar4()
	{
		return read_32(PCI_REG_BAR4);
	}

	void Function::set_bar4(uint32_t address)
	{
		write_32(PCI_REG_BAR4, address);
	}

	uint32_t Function::get_bar5()
	{
		return read_32(PCI_REG_BAR5);
	}

	void Function::set_bar5(uint32_t address)
	{
		write_32(PCI_REG_BAR5, address);
	}

	uint32_t Function::get_cardbus_cis_ptr()
	{
		return read_32(PCI_REG_CARDBUS_CIS_PTR);
	}

	uint16_t Function::get_subsystem_vendor_id()
	{
		return read_16(PCI_REG_SUBSYSTEM_VENDOR_ID);
	}

	uint16_t Function::get_subsystem_id()
	{
		return read_16(PCI_REG_SUBSYSTEM_ID);
	}

	uint32_t Function::get_expansion_rom_base_addr()
	{
		return read_16(PCI_REG_EXPANSION_ROM_BASE_ADDR);
	}

	uint16_t Function::get_capabilities_ptr()
	{
		return read_16(PCI_REG_CAPABILITIES_PTR);
	}

	uint8_t Function::get_interrupt_line()
	{
		return read_8(PCI_REG_INTERRUPT_LINE);
	}

	uint8_t Function::get_interrupt_pin()
	{
		return read_8(PCI_REG_INTERRUPT_PIN);
	}

	uint8_t Function::get_min_grant()
	{
		return read_8(PCI_REG_MIN_GRANT);
	}

	uint8_t Function::get_max_latency()
	{
		return read_8(PCI_REG_MAX_LATENCY);
	}

	void HostBridge::init()
	{
		// Start enumerating the whole PCI bus
		enumerate_bus(0);

		if (is_multifunction())
		{
			for (uint8_t function = 1; function < PCI_MAX_NUMBER_FUNCTIONS; function++)
			{
				auto device_function = Function(m_address.bus(), m_address.device(), function);

				if (!device_function.is_connected())
					continue;

				enumerate_bus(function);
			}
		}

#ifdef _DEBUG
		LibK::printf_debug_msg("Listing PCI devices:");
		kprintf("ADDRESS CLAS SUBC PROG VENDOR DEVICE HDRT\n");
		for (auto &device : m_all_pci_devices)
		{
			kprintf(
			    "%.2hhX:%.2hhX.%hhu 0x%.2hhx 0x%.2hhx 0x%.2hhx 0x%.4hx 0x%.4hx 0x%.2hhx\n",
			    device.get_address().bus(),
			    device.get_address().device(),
			    device.get_address().function(),
			    device.get_class(),
			    device.get_subclass(),
			    device.get_prog_if(),
			    device.get_vendor_id(),
			    device.get_device_id(),
			    device.get_header_type());
		}
#endif
	}

	void HostBridge::enumerate_bus(uint8_t bus)
	{
		for (uint8_t device = 0; device < PCI_MAX_NUMBER_DEVICES; device++)
			enumerate_device(bus, device);
	}

	void HostBridge::enumerate_device(uint8_t bus, uint8_t device)
	{
		auto base_device = Function(bus, device, 0);

		if (!base_device.is_connected())
			return;

		m_all_pci_devices.push_back(base_device);
		enumerate_function(bus, device, 0);

		if (base_device.is_multifunction())
		{
			for (uint8_t function = 1; function < PCI_MAX_NUMBER_FUNCTIONS; function++)
			{
				auto device_function = Function(bus, device, function);

				if (!device_function.is_connected())
					continue;

				m_all_pci_devices.push_back(device_function);

				enumerate_function(bus, device, function);
			}
		}
	}

	void HostBridge::enumerate_function(uint8_t bus, uint8_t device, uint8_t function)
	{
		auto device_function = Function(bus, device, function);

		auto base_class = device_function.get_class();
		auto subclass = device_function.get_subclass();

		if (base_class == PCI_CLASS_BRIDGE && subclass == PCI_BRIDGE_SUBCLASS_PCI_TO_PCI)
			enumerate_bus(function);
	}

	void HostBridge::enumerate(LibK::function<void(Function &)> callback)
	{
		for (auto &device : m_all_pci_devices)
			callback(device);
	}
} // namespace Kernel::PCI