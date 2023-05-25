#pragma once

#include <../../userland/libc/errno.h>

namespace Kernel::LibK
{
	template <typename T>
	class ErrorOr
	{
	public:
		ErrorOr(__ErrnoCode errno_code)
		    : m_errno_code(errno_code)
		{
		}

		ErrorOr(T value)
		    : m_value(value)
		{
		}

		[[nodiscard]] bool has_error() const { return m_errno_code != ESUCCESS; }
		[[nodiscard]] __ErrnoCode error() const { return m_errno_code; }
		[[nodiscard]] T data() const { return m_value; }

	private:
		T m_value{};
		__ErrnoCode m_errno_code{ESUCCESS};
	};

	template<>
	class ErrorOr<void>
	{
	public:
		ErrorOr() = default;

		ErrorOr(__ErrnoCode errno_code)
		    : m_errno_code(errno_code)
		{
		}

		[[nodiscard]] bool has_error() const { return m_errno_code != ESUCCESS; }
		[[nodiscard]] __ErrnoCode error() const { return m_errno_code; }

	private:
		__ErrnoCode m_errno_code{ESUCCESS};
	};
}