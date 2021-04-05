#ifndef KSTRING_H
#define KSTRING_H 1

#include <libk/kcmalloc.hpp>
#include <libk/kcstring.hpp>
#include <libk/kmath.hpp>
#include <libk/kvector.hpp>

namespace Kernel::LibK
{
	template <typename charT>
	class BasicString : Vector<charT>
	{
	public:
		BasicString() : Vector<charT>() {}

		BasicString(const charT *s)
		{
			assert(s);
			size_t len = strlen(s);

			this->m_capacity = next_power_of_two(len);
			this->m_size = len;
			this->m_array = (charT *)kmalloc(this->m_capacity, sizeof(charT));

			assert(this->m_array);
			memcpy(this->m_array, s, this->m_size);
		}

		const charT *c_str() const { return this->data(); }
	};

	typedef BasicString<char> String;
} // namespace Kernel::LibK

#endif // KSTRING_H