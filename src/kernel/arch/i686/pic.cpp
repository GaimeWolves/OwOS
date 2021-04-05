#include <arch/i686/pic.hpp>

#include <stddef.h>
#include <stdint.h>

#include <arch/io.hpp>

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

namespace Kernel::Processor
{
	static IO::Port masterCommand = IO::Port(0x20);
	static IO::Port masterData = masterCommand.offset(1);

	static IO::Port slaveCommand = IO::Port(0xA0);
	static IO::Port slaveData = masterCommand.offset(1);

	static inline void send_command(uint8_t cmd, bool slave);
	static inline void send_data(uint8_t data, bool slave);
	static inline uint8_t read_data(bool slave);

	static inline void send_command(uint8_t cmd, bool slave)
	{
		if (slave)
			slaveCommand.out(cmd);
		else
			masterCommand.out(cmd);
	}

	static inline void send_data(uint8_t data, bool slave)
	{
		if (slave)
			slaveData.out(data);
		else
			masterData.out(data);
	}

	static inline uint8_t read_data(bool slave)
	{
		if (slave)
			return slaveData.in<uint8_t>();
		else
			return masterData.in<uint8_t>();
	}

	void init_pic()
	{
		uint8_t icw = 0;

		// Begin initialization of PIC
		icw |= PIC_ICW1_DO_INIT;
		icw |= PIC_ICW1_EXPECT_IC4;

		send_command(icw, false);
		send_command(icw, true);

		// Base IRQ number (begin from IRQ 32)
		send_data(0x20, false);
		send_data(0x28, true);

		// Cascaded interrupt
		send_data(0x04, false); // IRQ 2 -> third bit
		send_data(0x02, true);  // IRQ 2 -> value in binary

		// Enable i86 mode
		icw = PIC_ICW4_UPM_86MODE;

		send_data(icw, false);
		send_data(icw, true);
	}

	void pic_eoi(int id)
	{
		if (id >= 0x28)
			send_command(PIC_OCW2_EOI, true);

		send_command(PIC_OCW2_EOI, false);
	}
} // namespace Kernel::Processor