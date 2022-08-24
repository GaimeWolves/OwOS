#include <tests.hpp>

#include "definitions.hpp"

#include <libk/kcstdarg.hpp>
#include <logging/logger.hpp>
#include <libk/kcstring.hpp>
#include <libk/kcstdio.hpp>

namespace Kernel::Tests
{
	static bool ok = true;

	void test_format(const char *fmt, const char *expected, ...)
	{
		char buffer[15] = {0};

		va_list ap;
		va_start(ap, expected);

		kvsprintf(buffer, fmt, ap);

		va_end(ap);

		if (strcmp(expected, buffer))
		{
			log(get_tag(false), "%s - Expected: %s Got: %s", fmt, expected, buffer);
			ok = false;
			return;
		}

		log(get_tag(true), "%s", fmt);
	}

	bool test_printf()
	{
		log("TEST", "printf");

		test_format("%c", "A", 'A');
		test_format("%10c", "         A", 'A');
		test_format("%*c", "         A", 10, 'A');

		test_format("%s", "hello printf", "hello printf");
		test_format("%.10s", "cut here:)", "cut here:):(");

		test_format("%u", "123", 123);
		test_format("%hhu", "123", (unsigned char)123);
		test_format("%llu", "123", (unsigned long long)123);
		test_format("%.10u", "0000000123", 123);
		test_format("%+.10u", "0000000123", 123);
		test_format("% .10u", "0000000123", 123);
		test_format("%010u", "0000000123", 123);
		test_format("%-10u:)", "123       :)", 123);

		test_format("%d", "-123", -123);
		test_format("%+d", "+123", 123);
		test_format("% d", " 123", 123);

		test_format("%x", "deadbeef", 0xdeadbeef);
		test_format("%X", "DEADBEEF", 0xdeadbeef);
		test_format("%#x", "0xdeadbeef", 0xdeadbeef);

		test_format("%o", "777", 0777);
		test_format("%#o", "0777", 0777);

		test_format("%p", "0xdeadbeef", 0xdeadbeef);
		test_format("%p", "(nil)", nullptr);

		test_format("%%", "%");

		int written = 0;
		test_format("12345%n", "12345", &written);
		test_format("%d", "5", written);

		return ok;
	}
} // namespace Kernel::Tests