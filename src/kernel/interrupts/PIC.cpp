#include <interrupts/PIC.hpp>

#include <arch/interrupts.hpp>
#include <interrupts/IRQHandler.hpp>

// Command Word 2 commands
#define PIC_OCW2_L1     0x01 // Level 1 interrupt level
#define PIC_OCW2_L2     0x02 // Level 2 interrupt level
#define PIC_OCW2_L3     0x04 // Level 3 interrupt level
#define PIC_OCW2_EOI    0x20 // End of Interrupt command
#define PIC_OCW2_SL     0x40 // Select command
#define PIC_OCW2_ROTATE 0x80 // Rotation command

// Initialization Control Word 1 values
#define PIC_ICW1_EXPECT_IC4          0x01
#define PIC_ICW1_NO_IC4              0x00
#define PIC_ICW1_IS_SINGLE           0x02
#define PIC_ICW1_IS_CASCADED         0x00
#define PIC_ICW1_ADI_CALLINTERVAL_4  0x04
#define PIC_ICW1_ADI_CALLINTERVAL_8  0x00
#define PIC_ICW1_LTIM_LEVELTRIGGERED 0x08
#define PIC_ICW1_LTIM_EDGETRIGGERED  0x00
#define PIC_ICW1_DO_INIT             0x10
#define PIC_ICW1_NO_INIT             0x00

// Initialization Control Word 4 values
#define PIC_ICW4_UPM_86MODE       0x01
#define PIC_ICW4_UPM_MCSMODE      0x00
#define PIC_ICW4_AUTO_EOI         0x02
#define PIC_ICW4_NO_AUTO_EOI      0x00
#define PIC_ICW4_BUFFER_MASTER    0x04
#define PIC_ICW4_BUFFER_SLAVE     0x00
#define PIC_ICW4_BUFFFERED        0x08
#define PIC_ICW4_NON_BUFFFERED    0x00
#define PIC_ICW4_SFNM_NESTED_MODE 0x10
#define PIC_ICW4_SFNM_NOT_NESTED  0x00

namespace Kernel::Interrupts
{
	PIC::PIC()
	{
		uint8_t icw = 0;

		// Begin initialization of PIC
		icw |= PIC_ICW1_DO_INIT;
		icw |= PIC_ICW1_EXPECT_IC4;

		send_command(icw, Master);
		send_command(icw, Slave);

		// Base IRQ number (begin from IRQ 32)
		send_data(Processor::FIRST_USABLE_INTERRUPT, Master);
		send_data(Processor::FIRST_USABLE_INTERRUPT + 8, Slave);

		// Cascaded interrupt
		send_data(0x04, Master); // IRQ 2 -> third bit
		send_data(0x02, Slave);  // IRQ 2 -> value in binary

		// Enable i86 mode
		icw = PIC_ICW4_UPM_86MODE;

		send_data(icw, Master);
		send_data(icw, Slave);

		disable();
	}

	void PIC::enable()
	{
		// Restore old mask
		send_data(m_interrupt_mask & 0xFF, Master);
		send_data(m_interrupt_mask >> 8, Slave);
	}

	void PIC::disable()
	{
		// Mask all interrupts
		send_data(0xFF, Master);
		send_data(0xFF, Slave);
	}

	bool PIC::eoi(IRQHandler &handler)
	{
		if (handler.original_interrupt_number() >= 16)
			return false;

		if (handler.original_interrupt_number() >= 8)
			send_command(PIC_OCW2_EOI, Slave);

		send_command(PIC_OCW2_EOI, Master);

		return true;
	}

	bool PIC::enable_interrupt(IRQHandler &handler)
	{
		if (handler.original_interrupt_number() >= 16)
			return false;

		m_interrupt_mask &= ~(1 << handler.original_interrupt_number());

		if (handler.original_interrupt_number() < 8)
			send_data(m_interrupt_mask & 0xFF, Master);
		else
			send_data(m_interrupt_mask >> 8, Slave);

		return true;
	}

	bool PIC::disable_interrupt(IRQHandler &handler)
	{
		if (handler.original_interrupt_number() >= 16)
			return false;

		m_interrupt_mask |= (1 << handler.original_interrupt_number());

		if (handler.original_interrupt_number() < 8)
			send_data(m_interrupt_mask & 0xFF, Master);
		else
			send_data(m_interrupt_mask >> 8, Slave);

		return true;
	}
}