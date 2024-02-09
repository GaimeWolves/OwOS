#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libk/kstring.hpp>

#include <arch/io.hpp>
#include <devices/CharacterDevice.hpp>

namespace Kernel
{
	class SerialDevice : public CharacterDevice
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

		static SerialDevice &get(size_t port);

		void configure(BaudRate rate, DataLength length, StopBits stopBits, ParityType parity);

		LibK::StringView name() override { return LibK::StringView(m_name); };

		[[nodiscard]] size_t size() override { return 0; }

		size_t read(size_t offset, size_t bytes, char *buffer) override;
		size_t write(size_t offset, size_t bytes, char *buffer) override;

		[[nodiscard]] bool is_faulty() const { return m_is_faulty; }

	protected:
		[[nodiscard]] bool can_open_for_read() const override { return true; };
		[[nodiscard]] bool can_open_for_write() const override { return true; };

	private:
		SerialDevice()
		    : CharacterDevice(4, 0)
		{
		}

		explicit SerialDevice(size_t port)
			: CharacterDevice(4, port) // TODO: Do major number allocation (for now we hardcode the numbers linux uses)
		{
			m_data_reg.set(s_io_ports[port]);
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
			m_is_faulty = false;

			// Todo: This should be handled by /proc/devices using the (future) device manager
			m_name = LibK::string("ttyS");
			m_name += '0' + port;

			configure(BaudRate::B115200, DataLength::L8, StopBits::S1, ParityType::NONE);
		}

		void test();

		static constexpr size_t s_io_ports[4]{
		    0x3f8,
		    0x2f8,
		    0x3e8,
		    0x2e8,
		};

		IO::Port m_data_reg{};
		IO::Port m_int_enable_reg{};
		IO::Port m_baud_rate_low_reg{};
		IO::Port m_baud_rate_high_reg{};
		IO::Port m_int_ident_reg{};
		IO::Port m_fifo_ctl_reg{};
		IO::Port m_line_ctl_reg{};
		IO::Port m_modem_ctl_reg{};
		IO::Port m_line_status_reg{};
		IO::Port m_modem_status_reg{};
		IO::Port m_scratch_reg{};

		bool m_is_faulty{false};
		LibK::string m_name{};

		static SerialDevice s_instances[4];
	};
} // namespace Kernel::Devices
