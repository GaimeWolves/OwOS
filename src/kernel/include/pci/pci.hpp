#ifndef PCI_H
#define PCI_H 1

#include <stddef.h>
#include <stdint.h>

#include <pci/definitions.hpp>

#include <libk/kvector.hpp>

namespace Kernel::PCI
{
	enum class Type : uint8_t
	{
		Standard = PCI_HEADER_STANDARD,
		PCI_to_PCI = PCI_HEADER_PCI_BRIDGE,
		PCI_to_CardBus = PCI_HEADER_CARDBUS_BRIDGE,
	};

	class Function
	{
	public:
		Function(uint8_t bus, uint8_t device, uint8_t function)
		    : m_address(Address(bus, device, function))
		{
			if (get_vendor_id() == PCI_DEVICE_DISCONNECTED)
			{
				m_connected = false;
				return;
			}

			m_connected = true;

			uint8_t header_type = get_header_type();
			m_type = (Type)(header_type & PCI_HEADER_TYPE_MASK);

			if (function > 0)
				m_multifunction = true;
			else
				m_multifunction = (header_type & PCI_HEADER_IS_MULTIFUNCTION) != 0;
		}

		const Address &get_address() const { return m_address; }

		bool is_multifunction() const { return m_multifunction; }
		bool is_connected() const { return m_connected; }
		Type get_type() const { return m_type; }

		uint32_t read_32(uint8_t offset);
		uint16_t read_16(uint8_t offset);
		uint8_t read_8(uint8_t offset);

		void write_32(uint8_t offset, uint32_t value);
		void write_16(uint8_t offset, uint16_t value);
		void write_8(uint8_t offset, uint8_t value);

		uint16_t get_device_id();
		uint16_t get_vendor_id();

		uint16_t get_status();
		void set_command(uint16_t command);

		uint8_t get_revision_id();

		uint8_t get_prog_if();

		uint8_t get_subclass();
		uint8_t get_class();

		void set_cache_line_size(uint8_t size);

		uint8_t get_latency();

		uint8_t get_header_type();

		uint8_t get_bist();
		void set_bist(uint8_t value);

		uint32_t get_bar0();
		void set_bar0(uint32_t address);

		uint32_t get_bar1();
		void set_bar1(uint32_t address);

		uint32_t get_bar2();
		void set_bar2(uint32_t address);

		uint32_t get_bar3();
		void set_bar3(uint32_t address);

		uint32_t get_bar4();
		void set_bar4(uint32_t address);

		uint32_t get_bar5();
		void set_bar5(uint32_t address);

		uint32_t get_cardbus_cis_ptr();

		uint16_t get_subsystem_vendor_id();
		uint16_t get_subsystem_id();

		uint32_t get_expansion_rom_base_addr();

		uint16_t get_capabilities_ptr();

		uint8_t get_interrupt_line();
		uint8_t get_interrupt_pin();

		uint8_t get_min_grant();

		uint8_t get_max_latency();

	protected:
		Address m_address;

		Type m_type{Type::Standard};
		bool m_multifunction{false};
		bool m_connected{false};
	};

	class HostBridge final : private Function
	{
	public:
		static HostBridge &instance()
		{
			static HostBridge *instance{nullptr};

			if (!instance)
				instance = new HostBridge();

			return *instance;
		}

		HostBridge(HostBridge &) = delete;
		void operator=(const HostBridge &) = delete;

		void init();

	private:
		HostBridge() : Function(0, 0, 0)
		{
		}

		~HostBridge() = default;

		void enumerate_bus(uint8_t bus);
		void enumerate_device(uint8_t bus, uint8_t device);
		void enumerate_function(uint8_t bus, uint8_t device, uint8_t function);

		LibK::Vector<Function> m_all_pci_devices;
	};
} // namespace Kernel::PCI

#endif // PCI_H