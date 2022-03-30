#pragma once

#include <stddef.h>
#include <stdint.h>

#include <arch/io.hpp>

#include <libk/kstring.hpp>

namespace Kernel::Devices
{
	class SerialPort
	{
	public:
		enum class BaudRate : uint16_t
		{
			B115200 = 1,
			B57600 = 2,
			B38400 = 3,
			B19200 = 6,
			B14400 = 8,
			B9600 = 12,
			B7200 = 16,
			B4800 = 24,
			B2400 = 48,
			B1800 = 64,
			B1200 = 96,
			B600 = 192,
			B300 = 384,
			B150 = 768,
			B75 = 1536,
		};

		enum class DataLength : uint8_t
		{
			L5 = 0b00,
			L6 = 0b01,
			L7 = 0b10,
			L8 = 0b11,
		};

		enum class StopBits : uint8_t
		{
			S1 = 0b000,
			S2 = 0b100,
		};

		enum class ParityType : uint8_t
		{
			NONE = 0b000,
			ODD = 0b001,
			EVEN = 0b011,
			MARK = 0b101,
			SPACE = 0b111,
		};

		enum Port : size_t
		{
			COM1 = 0,
			COM2 = 1,
			COM3 = 2,
			COM4 = 3,
		};

		static void init();
		static bool is_initialized();
		static SerialPort &get(size_t port);

		void configure(BaudRate rate, DataLength length, StopBits stopBits, ParityType parity);

		void write(const char *str);
		void write_one(char ch);

		LibK::string read(size_t n);
		char read_one();

		bool is_faulty() const { return m_is_faulty; }

	private:
		IO::Port m_data_reg;
		IO::Port m_int_enable_reg;
		IO::Port m_baud_rate_low_reg;
		IO::Port m_baud_rate_high_reg;
		IO::Port m_int_ident_reg;
		IO::Port m_fifo_ctl_reg;
		IO::Port m_line_ctl_reg;
		IO::Port m_modem_ctl_reg;
		IO::Port m_line_status_reg;
		IO::Port m_modem_status_reg;
		IO::Port m_scratch_reg;

		bool m_is_faulty;

		SerialPort() = default;

		SerialPort(size_t port)
		{
			m_data_reg.set(m_io_ports[port]);
			m_int_enable_reg = m_data_reg.offset(1);
			m_baud_rate_low_reg = m_data_reg.offset(0);
			m_baud_rate_high_reg = m_data_reg.offset(1);
			m_int_ident_reg = m_data_reg.offset(2);
			m_fifo_ctl_reg = m_data_reg.offset(2);
			m_line_ctl_reg = m_data_reg.offset(3);
			m_modem_ctl_reg = m_data_reg.offset(4);
			m_line_status_reg = m_data_reg.offset(5);
			m_modem_status_reg = m_data_reg.offset(6);
			m_scratch_reg = m_data_reg.offset(7);

			configure(BaudRate::B115200, DataLength::L8, StopBits::S1, ParityType::NONE);
		}

		void test();

		static constexpr size_t m_io_ports[4]{
		    0x3f8,
		    0x2f8,
		    0x3e8,
		    0x2e8,
		};

		static SerialPort m_instances[4];
	};
} // namespace Kernel::Devices
