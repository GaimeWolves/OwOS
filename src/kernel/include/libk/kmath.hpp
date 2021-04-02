#ifndef KMATH_H
#define KMATH_H 1

#include <limits.h>

namespace Kernel::LibK
{

    template <typename T, typename V>
    auto min(T a, V b) -> decltype(a + b)
    {
        using type = decltype(a + b);
        if (static_cast<type>(a) < static_cast<type>(b))
            return a;

        return b;
    }

    template <typename T, typename V>
    auto max(T a, V b) -> decltype(a + b)
    {
        using type = decltype(a + b);
        if (static_cast<type>(a) > static_cast<type>(b))
            return a;

        return b;
    }

    template <typename T>
    T next_power_of_two(T value)
    {
        --value;
        for (size_t i = 1; i < sizeof(T) * CHAR_BIT; i *= 2)
            value |= value >> i;
        return value + 1;
    }

    template <typename T>
    constexpr T round_up_to_power_of_two(T value, T power_of_two)
    {
        return ((value - 1) & ~(power_of_two - 1)) + power_of_two;
    }

    template <typename T>
    constexpr T round_down_to_power_of_two(T value, T power_of_two)
    {
        return value & power_of_two;
    }

} // namespace Kernel::LibK

#endif // KMATH_H