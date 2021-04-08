#include <devices/SerialPort.hpp>

// Interrupt Enable register bits
#define INT_ENABLE_ERBFI 0b00000001 // Enable received data available interrupt
#define INT_ENABLE_ETBEI 0b00000010 // Enable transmitter holding register empty interrupt
#define INT_ENABLE_ELSI  0b00000100 // Enable receiver line status interrupt
#define INT_ENABLE_EDSSI 0b00001000 // Enable MODEM status interrupt

// Interrupt identification register bits
#define INT_IDENT_PENDING      0b00000001 // Zero if an interrupt is pending
#define INT_IDENT_ID_MASK      0b00001110 // Interrupt ID
#define INT_IDENT_FIFO_ENABLED 0b11000000 // FIFOs enabled

// FIFO control register bits
#define FIFO_CTL_ENABLE     0b00000001 // Enable FIFOs
#define FIFO_CTL_RCVR_RESET 0b00000010 // Reset receiver FIFO
#define FIFO_CTL_XMIT_RESET 0b00000100 // Reset transmitter FIFO
#define FIFO_CTL_DMA_SELECT 0b00001000 // Select DMA mode
#define FIFO_CTL_RCVR_01    0b00000000 // Receiver FIFO trigger level in bytes
#define FIFO_CTL_RCVR_04    0b01000000
#define FIFO_CTL_RCVR_08    0b10000000
#define FIFO_CTL_RCVR_14    0b11000000

// Line control register bits
#define LINE_CTL_SET_BREAK 0b01000000
#define LINE_CTL_DLAB      0b10000000 // Divisor Latch Access Bit

// MODEM control register bits
#define MODEM_CTL_DTR   0b00000001 // Data terminal ready
#define MODEM_CTL_RTS   0b00000010 // Request to send
#define MODEM_CTL_OUT_1 0b00000100
#define MODEM_CTL_OUT_2 0b00001000
#define MODEM_CTL_LOOP  0b00010000 // Loopback mode

// Line status register bits
#define LINE_STATUS_DR   0b00000001 // Data ready
#define LINE_STATUS_OE   0b00000010 // Overrun error
#define LINE_STATUS_PE   0b00000100 // Parity error
#define LINE_STATUS_FE   0b00001000 // Framing error
#define LINE_STATUS_BI   0b00010000 // Break interrupt
#define LINE_STATUS_THRE 0b00100000 // Transmitter holding register empty
#define LINE_STATUS_TEMT 0b01000000 // Transmitter empty
#define LINE_STATUS_RCVR 0b10000000 // Error in receiver FIFO

// MODEM status register bits
#define MODEM_STATUS_DCTS 0b00000001 // Delta clear to send
#define MODEM_STATUS_DDSR 0b00000010 // Delta data set ready
#define MODEM_STATUS_TERI 0b00000100 // Traling edge ring indicator
#define MODEM_STATUS_DDCD 0b00001000 // Delta data carrier detect
#define MODEM_STATUS_CTS  0b00010000 // Clear to send
#define MODEM_STATUS_DSR  0b00100000 // Data set ready
#define MODEM_STATUS_RI   0b01000000 // Ring indicator
#define MODEM_STATUS_DCD  0b10000000 // Data carrier detect

namespace Kernel::Devices
{
	SerialPort SerialPort::m_instances[];

	static inline bool can_send(IO::Port &line_status);
	static inline bool can_receive(IO::Port &line_status);

	static inline bool can_send(IO::Port &line_status)
	{
		return (line_status.in<uint8_t>() & LINE_STATUS_THRE) != 0;
	}

	static inline bool can_receive(IO::Port &line_status)
	{
		return (line_status.in<uint8_t>() & LINE_STATUS_DR) != 0;
	}

	void SerialPort::init()
	{
		for (size_t i = 0; i < 4; i++)
			m_instances[i] = SerialPort(i);
	}

	SerialPort &SerialPort::get(size_t port)
	{
		return m_instances[port];
	}

	void SerialPort::configure(BaudRate rate, DataLength length, StopBits stopBits, ParityType parity)
	{
		// Disable interrupts
		m_int_enable_reg.out<uint8_t>(0);

		// Set DLAB (set baud rate divisor)
		uint16_t rateValue = (uint16_t)rate;
		m_line_ctl_reg.out<uint8_t>(LINE_CTL_DLAB);
		m_baud_rate_low_reg.out((uint8_t)(rateValue & 0xFF));
		m_baud_rate_high_reg.out((uint8_t)(rateValue >> 8));

		// Set protocol
		uint8_t protocol = (uint8_t)length | (uint8_t)stopBits | (uint8_t)parity;
		m_line_ctl_reg.out(protocol);

		// Enable FIFOs, reset them and set them to a 14-byte threshold
		m_fifo_ctl_reg.out<uint8_t>(FIFO_CTL_RCVR_14 | FIFO_CTL_ENABLE | FIFO_CTL_RCVR_RESET | FIFO_CTL_XMIT_RESET);

		test();

		if (!m_is_faulty)
		{
			// Enable modem with OUT#1, OUT#2, RTS and DTR bits enabled
			m_modem_ctl_reg.out<uint8_t>(MODEM_CTL_OUT_1 | MODEM_CTL_OUT_2 | MODEM_CTL_RTS | MODEM_CTL_DTR);
		}
	}

	void SerialPort::test()
	{
		// Set modem in loopback mode with OUT#2 and RTS bits enabled
		m_modem_ctl_reg.out<uint8_t>(MODEM_CTL_LOOP | MODEM_CTL_OUT_2 | MODEM_CTL_RTS);

		// Send test byte and check it immediatly after
		m_data_reg.out<uint8_t>(0xAE);
		m_is_faulty = m_data_reg.in<uint8_t>() != 0xAE;
	}

	void SerialPort::write(const LibK::string &str)
	{
		if (m_is_faulty)
			return;

		for (size_t i = 0; i < str.size(); i++)
			write_one(str[i]);
	}

	void SerialPort::write_one(char ch)
	{
		if (m_is_faulty)
			return;

		while (!can_send(m_line_status_reg))
			;

		m_data_reg.out(ch);
	}

	LibK::string SerialPort::read(size_t n)
	{
		LibK::string ret;

		if (m_is_faulty)
			return ret;

		while (n--)
			ret.push_back(read_one());

		return ret;
	}

	char SerialPort::read_one()
	{
		if (m_is_faulty)
			return -1;

		while (!can_receive(m_line_status_reg))
			;

		return m_data_reg.in<char>();
	}
} // namespace Kernel::Devices