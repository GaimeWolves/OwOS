#include <time/PIT.hpp>

#include <time/EventManager.hpp>

#define CMD_CHANNEL_0 (0b00 << 6)
#define CMD_CHANNEL_1 (0b01 << 6)
#define CMD_CHANNEL_2 (0b10 << 6)
#define CMD_READ_BACK (0b11 << 6)

#define CMD_ACCESS_LATCH_COUNT (0b00 << 4)
#define CMD_ACCESS_LOW_BYTE    (0b01 << 4)
#define CMD_ACCESS_HIGH_BYTE   (0b01 << 4)
#define CMD_ACCESS_FULL_WORD   (0b11 << 4)

#define CMD_MODE_INT         (0b000 << 1)
#define CMD_MODE_HW_ONE_SHOT (0b001 << 1)
#define CMD_MODE_RATE        (0b010 << 1)
#define CMD_MODE_SQUARE_WAVE (0b011 << 1)
#define CMD_MODE_SW_STROBE   (0b100 << 1)
#define CMD_MODE_HW_STROBE   (0b101 << 1)

#define CMD_MODE_BINARY (0)
#define CMD_MODE_BCD (1)

namespace Kernel::Time
{
	void PIT::start(uint64_t interval)
	{
		assert(interval <= 65536 && interval > 0);

		uint8_t command = CMD_CHANNEL_0 | CMD_ACCESS_FULL_WORD | CMD_MODE_BINARY | CMD_MODE_INT;

		if (interval == 65536)
			interval = 0;

		m_command_register.out(command);
		m_channel_0_data.out((uint8_t)(interval & 0xFF));
		m_channel_0_data.out((uint8_t)((interval >> 8) & 0xFF));

		enable_irq();
	}

	void PIT::stop()
	{
		disable_irq();
	}

	void PIT::handle_interrupt(const CPU::registers_t &regs __unused)
	{
		m_callback();
	}
} // namespace Kernel::Time