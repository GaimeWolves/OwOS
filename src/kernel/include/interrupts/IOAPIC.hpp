#pragma once

#include <libk/kvector.hpp>

#include <interrupts/InterruptController.hpp>
#include <interrupts/IRQHandler.hpp>
#include <interrupts/definitions.hpp>
#include <common_attributes.h>

namespace Kernel::Interrupts
{
	class IOAPIC final : public InterruptController
	{
	public:
		explicit IOAPIC(uintptr_t address, uint32_t gsi_base);

		void add_redirection(madt_ioapic_override_entry_t redirection);
		void set_nmi(madt_ioapic_nmi_entry_t nmi);

		void enable() override;
		void disable() override;

		bool eoi(IRQHandler &handler) override;

		bool enable_interrupt(IRQHandler &handler) override;
		bool disable_interrupt(IRQHandler &handler) override;

		[[nodiscard]] uint32_t get_gsi_base() const override { return m_gsi_base; };
		[[nodiscard]] uint32_t get_gsi_count() const override { return m_gsi_count; };

	private:
		always_inline uint32_t read_register(uint32_t reg)
		{
			*((uint32_t volatile *) m_register_address) = reg;
			return *((uint32_t volatile *)(m_register_address + 0x10));
		}

		always_inline void write_register(uint32_t reg, uint32_t data)
		{
			*((uint32_t volatile *) m_register_address) = reg;
			*((uint32_t volatile *)(m_register_address + 0x10)) = data;
		}

		uintptr_t m_register_address{0};
		uint32_t m_gsi_base{0};
		uint32_t m_gsi_count{0};
		uint32_t *m_interrupt_mask{nullptr};
		LibK::vector<madt_ioapic_override_entry_t> m_redirections;
	};
}