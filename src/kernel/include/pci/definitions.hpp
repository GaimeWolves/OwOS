#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libk/kcassert.hpp>

#define PCI_HEADER_IS_MULTIFUNCTION 0b10000000
#define PCI_HEADER_TYPE_MASK        0b00000011
#define PCI_HEADER_STANDARD         0b00000000
#define PCI_HEADER_PCI_BRIDGE       0b00000001
#define PCI_HEADER_CARDBUS_BRIDGE   0b00000010

#define PCI_DEVICE_DISCONNECTED 0xFFFF

#define PCI_CONFIG_ADDR_IO_PORT 0xCF8
#define PCI_CONFIG_DATA_IO_PORT 0xCFC

#define PCI_MAX_NUMBER_BUSSES    256
#define PCI_MAX_NUMBER_DEVICES   32
#define PCI_MAX_NUMBER_FUNCTIONS 8

// For now only the standard header layout is defined here
#define PCI_REG_VENDOR_ID               0x00
#define PCI_REG_DEVICE_ID               0x02
#define PCI_REG_COMMAND                 0x04
#define PCI_REG_STATUS                  0x06
#define PCI_REG_REVISION_ID             0x08
#define PCI_REG_PROG_IF                 0x09
#define PCI_REG_SUBCLASS                0x0A
#define PCI_REG_CLASS                   0x0B
#define PCI_REG_CACHE_LINE_SIZE         0x0C
#define PCI_REG_LATENCY_TIMER           0x0D
#define PCI_REG_HEADER_TYPE             0x0E
#define PCI_REG_BIST                    0x0F
#define PCI_REG_BAR0                    0x10
#define PCI_REG_BAR1                    0x14
#define PCI_REG_BAR2                    0x18
#define PCI_REG_BAR3                    0x1C
#define PCI_REG_BAR4                    0x20
#define PCI_REG_BAR5                    0x24
#define PCI_REG_CARDBUS_CIS_PTR         0x28
#define PCI_REG_SUBSYSTEM_VENDOR_ID     0x2C
#define PCI_REG_SUBSYSTEM_ID            0x2E
#define PCI_REG_EXPANSION_ROM_BASE_ADDR 0x30
#define PCI_REG_CAPABILITIES_PTR        0x34
#define PCI_REG_INTERRUPT_LINE          0x3C
#define PCI_REG_INTERRUPT_PIN           0x3D
#define PCI_REG_MIN_GRANT               0x3E
#define PCI_REG_MAX_LATENCY             0x3F

#define PCI_CLASS_BRIDGE               0x06
#define PCI_BRIDGE_SUBCLASS_PCI_TO_PCI 0x04

namespace Kernel::PCI
{
	class Address
	{
	public:
		Address() = default;

		Address(uint8_t bus, uint8_t device, uint8_t function)
		    : m_bus(bus), m_device(device), m_function(function)
		{
			assert(m_device < PCI_MAX_NUMBER_DEVICES);
			assert(m_function < PCI_MAX_NUMBER_FUNCTIONS);
		}

		uint32_t address(uint8_t reg_offset) { return address(m_bus, m_device, m_function, reg_offset); }
		uint32_t operator()(uint8_t reg_offset) { return address(reg_offset); }

		static uint32_t address(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset)
		{
			assert(device < PCI_MAX_NUMBER_DEVICES);
			assert(function < PCI_MAX_NUMBER_FUNCTIONS);

			return 0x80000000 |
			       (((uint32_t)bus) << 16) |
			       (((uint32_t)device & 0x1F) << 11) |
			       (((uint32_t)function & 0x07) << 8) |
			       ((uint32_t)reg_offset & 0xFC);
		}

		uint8_t bus() const { return m_bus; }
		uint8_t device() const { return m_device; }
		uint8_t function() const { return m_function; }

	private:
		uint8_t m_bus{0};
		uint8_t m_device{0};
		uint8_t m_function{0};
	};
} // namespace Kernel::PCI
