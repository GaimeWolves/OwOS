#ifndef PRINTF_H
#define PRINTF_H 1

#include <stdint.h>
#include <stddef.h>

#include <libk/kstdarg.hpp>
#include <libk/kctype.hpp>
#include <libk/kmath.hpp>
#include <libk/kstring.hpp>
#include <libk/kutility.hpp>

namespace Kernel::LibK
{

    namespace __Printf
    {

        enum class Length
        {
            hh,
            h,
            None,
            l,
            ll,
            j,
            z,
            t,
            L,
        };

        typedef struct state_t
        {
            char specifier;
            Length length;
            bool left_justify;
            bool force_sign;
            bool space_sign;
            bool alternative;
            bool pad_zeroes;
            bool has_width;
            bool has_precision;
            int width;
            int precision;
        } state_t;

        inline void parse_flags(state_t &state, const char *&fmt)
        {
            while (*fmt)
            {
                switch (*fmt)
                {
                case '-':
                    if (state.left_justify)
                        return;
                    state.left_justify = true;
                    break;
                case '+':
                    if (state.force_sign)
                        return;
                    state.force_sign = true;
                    break;
                case ' ':
                    if (state.space_sign)
                        return;
                    state.space_sign = true;
                    break;
                case '#':
                    if (state.alternative)
                        return;
                    state.alternative = true;
                    break;
                case '0':
                    if (state.pad_zeroes)
                        return;
                    state.pad_zeroes = true;
                    break;

                default:
                    return;
                }

                fmt++;
            }
        }

        inline void parse_length(state_t &state, const char *&fmt)
        {
            switch (*fmt)
            {
            case 'h':
                if (*(fmt + 1) == 'h')
                {
                    state.length = Length::hh;
                    fmt += 2;
                    break;
                }

                state.length = Length::h;
                fmt++;
                break;
            case 'l':
                if (*(fmt + 1) == 'l')
                {
                    state.length = Length::ll;
                    fmt += 2;
                    break;
                }

                state.length = Length::l;
                fmt++;
                break;
            case 'j':
                state.length = Length::j;
                fmt++;
                break;
            case 't':
                state.length = Length::t;
                fmt++;
                break;
            case 'z':
                state.length = Length::z;
                fmt++;
                break;
            case 'L':
                state.length = Length::L;
                fmt++;
                break;

            default:
                state.length = Length::None;
                break;
            }
        }

        void parse_number(state_t &state, char *buffer, uint64_t num, size_t base, bool is_ptr = false)
        {
            static const char *alphabet = "0123456789abcdef";
            size_t iteration = 0;
            size_t written = 0;

            char *current = buffer;

            while (!is_ptr || iteration < sizeof(void *) * 2)
            {
                if (!num && !is_ptr)
                {
                    if (!state.pad_zeroes)
                        break;

                    if (state.has_precision && written >= (size_t)state.precision)
                        break;

                    if (written > 0 || (written == 0 && state.has_precision && state.precision == 0))
                        break;
                }

                char ch = alphabet[num % base];

                *current++ = ch;
                written++;

                num /= base;
                iteration++;
            }

            strrev(buffer);
        }

        template <typename putc_func>
        int print_value(state_t &state, putc_func putc, char *&buffer, const char *str)
        {
            int written = 0;
            size_t len = strlen(str);

            char sign_char = 0;

            if (str[0] == '-')
            {
                sign_char = *str++;
                len--;
            }
            else if (state.specifier == 'd' || state.specifier == 'i')
            {
                if (state.force_sign)
                    sign_char = '+';
                else if (state.space_sign)
                    sign_char = ' ';
            }

            size_t zero_length = (size_t)state.precision > len ? state.precision - len : 0;
            size_t pad_length = (size_t)state.width > len + zero_length ? state.width - len - zero_length : 0;

            if (!state.left_justify)
            {
                if (pad_length > 0 && state.pad_zeroes && sign_char)
                {
                    pad_length--;
                    putc(buffer, sign_char);

                    written++;
                }

                for (size_t i = 0; i < pad_length; i++)
                    putc(buffer, state.pad_zeroes ? '0' : ' ');

                written += pad_length;
            }

            if (sign_char)
            {
                if (zero_length > 0)
                    zero_length--;

                putc(buffer, sign_char);
                written++;
            }

            for (size_t i = 0; i < zero_length; i++)
                putc(buffer, '0');

            written += zero_length;

            for (size_t i = 0; i < len; i++)
                putc(buffer, str[i]);

            written += len;

            if (state.left_justify)
            {
                for (size_t i = 0; i < pad_length; i++)
                    putc(buffer, ' ');

                written += pad_length;
            }

            return written;
        }

        template <typename putc_func>
        int print_string(state_t &state, putc_func putc, char *&buffer, const char *str)
        {
            size_t len = strlen(str);

            if (state.has_precision)
                len = min(len, state.precision);

            size_t pad_length = (size_t)state.width > len ? state.width - len : 0;

            if (!state.left_justify)
            {
                for (size_t i = 0; i < pad_length; i++)
                    putc(buffer, ' ');
            }

            for (size_t i = 0; i < len; i++)
                putc(buffer, str[i]);

            if (state.left_justify)
            {
                for (size_t i = 0; i < pad_length; i++)
                    putc(buffer, ' ');
            }

            return len + pad_length;
        }

        template <typename putc_func, typename argument_list_ref_t, template <typename T, typename V = argument_list_ref_t> typename next_argument>
        struct printf_impl_t
        {
            inline __attribute__((always_inline)) printf_impl_t(putc_func &putc, char *&buffer, const int &written, state_t &state, argument_list_ref_t ap)
                : m_putc(putc), m_buffer(buffer), m_written(written), m_state(state), m_ap(ap)
            {
            }

            bool parse_width_or_precision(const char *&fmt, int &ref)
            {
                int width = 0;

                if (isdigit(*fmt))
                {
                    while (isdigit(*fmt))
                    {
                        width *= 10;
                        width += *fmt - '0';
                        fmt++;
                    }
                }
                else if (*fmt == '*')
                {
                    width = next_argument<int>()(m_ap);
                    fmt++;
                }
                else
                    return false;

                ref = width;
                return true;
            }

            int handle_octal()
            {
                uint64_t value = 0;

                if (m_state.length == Length::ll)
                    value = next_argument<unsigned long long>()(m_ap);
                else
                    value = next_argument<unsigned int>()(m_ap);

                char buf[24]{0};

                if (m_state.alternative && value > 0)
                {
                    buf[0] = '0';

                    parse_number(m_state, buf + 1, value, 8);
                }
                else
                    parse_number(m_state, buf, value, 8);

                return print_value(m_state, m_putc, m_buffer, buf);
            }

            int handle_hex()
            {
                uint64_t value = 0;

                if (m_state.length == Length::ll)
                    value = next_argument<unsigned long long>()(m_ap);
                else
                    value = next_argument<unsigned int>()(m_ap);

                char buf[19]{0};

                if (m_state.alternative && value > 0)
                {
                    buf[0] = '0';
                    buf[1] = 'x';

                    parse_number(m_state, buf + 2, value, 16);
                }
                else
                    parse_number(m_state, buf, value, 16);

                if (m_state.specifier == 'X')
                {
                    for (size_t i = 0; i < sizeof(buf) / sizeof(char); i++)
                        buf[i] = toupper(buf[i]);
                }

                return print_value(m_state, m_putc, m_buffer, buf);
            }

            int handle_unsigned()
            {
                uint64_t value = 0;

                if (m_state.length == Length::ll)
                    value = next_argument<unsigned long long>()(m_ap);
                else
                    value = next_argument<unsigned int>()(m_ap);

                char buf[21]{0};
                parse_number(m_state, buf, value, 10);
                return print_value(m_state, m_putc, m_buffer, buf);
            }

            int handle_signed()
            {
                int64_t value = 0;

                if (m_state.length == Length::ll)
                    value = next_argument<long long>()(m_ap);
                else
                    value = next_argument<int>()(m_ap);

                char buf[22]{0};

                if (value < 0)
                {
                    buf[0] = '-';
                    parse_number(m_state, buf + 1, (uint64_t)-value, 10);
                }
                else
                    parse_number(m_state, buf, (uint64_t)value, 10);

                return print_value(m_state, m_putc, m_buffer, buf);
            }

            int handle_n()
            {
                // TODO: Check flags

                void *n_ref = next_argument<void *>()(m_ap);

                if (m_state.length == Length::ll)
                    *(int64_t *)n_ref = m_written;
                else
                    *(int32_t *)n_ref = m_written;

                return 0;
            }

            int handle_p()
            {
                uintptr_t ptr = (uintptr_t)next_argument<void *>()(m_ap);

                if (!ptr)
                    return print_string(m_state, m_putc, m_buffer, "(nil)");

                char buf[19]{'0', 'x'};
                parse_number(m_state, buf + 2, ptr, 16, true);
                return print_value(m_state, m_putc, m_buffer, buf);
            }

            int handle_s()
            {
                const char *str = next_argument<const char *>()(m_ap);
                return print_string(m_state, m_putc, m_buffer, str);
            }

            int handle_c()
            {
                char c = next_argument<int>()(m_ap);
                char str[] = {c, 0}; // ensure null-termination
                return print_string(m_state, m_putc, m_buffer, str);
            }

            int handle_specifier()
            {
                switch (m_state.specifier)
                {
                case 'c':
                    return handle_c();

                case 's':
                    return handle_s();

                case 'd':
                case 'i':
                    return handle_signed();

                case 'u':
                    return handle_unsigned();

                case 'x':
                case 'X':
                    return handle_hex();

                case 'o':
                    return handle_octal();

                case 'p':
                    return handle_p();

                case 'n':
                    return handle_n();

                default:
                    return 0;
                }
            }

        protected:
            putc_func &m_putc;
            char *&m_buffer;
            const int &m_written;
            state_t &m_state;
            argument_list_ref_t m_ap;
        };

        template <typename putc_func, template <typename T, typename U, template <typename X, typename Y> typename V> typename impl_t = printf_impl_t, typename argument_list_t = va_list, template <typename T, typename V = decltype(declval<argument_list_t &>())> typename next_argument = va_arg_next_argument>
        int printf_internal(putc_func putc, char *buffer, const char *fmt, argument_list_t ap)
        {
            int ret = 0;

            state_t state;
            impl_t<putc_func, argument_list_t &, next_argument> impl{putc, buffer, ret, state, ap};

            for (; *fmt; fmt++)
            {
                if (*fmt == '%')
                {
                    state = {0, Length::None, false, false, false, false, false, false, false, 0, 0};

                    fmt++;

                    if (*fmt == '%')
                    {
                        putc(buffer, '%');
                        ret++;
                        continue;
                    }

                    parse_flags(state, fmt);
                    state.has_width = impl.parse_width_or_precision(fmt, state.width);

                    if (*fmt == '.')
                    {
                        state.has_precision = true;
                        fmt++;

                        impl.parse_width_or_precision(fmt, state.precision);
                    }

                    parse_length(state, fmt);

                    state.specifier = *fmt;

                    ret += impl.handle_specifier();
                    continue;
                }

                putc(buffer, *fmt);
                ret++;
            }

            return ret;
        }

    } // namespace __Printf

    using __Printf::printf_internal;

} // namespace Kernel::LibK

#endif // PRINTF_H