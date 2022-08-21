#pragma once

#include <stddef.h>

#include <libk/kcassert.hpp>
#include <libk/__iterators.hpp>
#include <libk/kstring.hpp>
#include <libk/ArrayView.hpp>

namespace Kernel::LibK
{
	class StringView : public ArrayView<char>
	{
	public:
		StringView() = default;

		explicit StringView(const LibK::string &string)
		    : ArrayView<char>(string.c_str(), string.size())
		{
		}

		explicit StringView(const char *string)
			: ArrayView<char>(string, strlen(string))
		{
		}

		[[nodiscard]] const char *c_str() const { return this->data(); }
	};
}