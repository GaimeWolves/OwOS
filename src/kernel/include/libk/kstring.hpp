#pragma once

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>
#include <libk/kvector.hpp>

namespace Kernel::LibK
{
	template <typename charT>
	class basic_string : public vector<charT>
	{
	public:
		basic_string()
		    : vector<charT>()
		{
		}

		basic_string(const charT *s)
			: vector<charT>(strlen(s))
		{
			assert(s);
			assert(this->m_array);

			memcpy(this->m_array, s, this->m_size);
		}

		const charT *c_str() const { return this->data(); }

		basic_string &operator+=(charT c)
		{
			this->push_back(c);
			return *this;
		}
	};

	typedef basic_string<char> string;
} // namespace Kernel::LibK
