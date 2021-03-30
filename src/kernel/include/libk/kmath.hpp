#ifndef KMATH_H
#define KMATH_H 1

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

} // namespace Kernel::LibK

#endif // KMATH_H