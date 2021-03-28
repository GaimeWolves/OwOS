#include <tests.hpp>

#include <libk/kstdio.hpp>

namespace Kernel::Tests
{

    static bool constructor_was_run = false;

    __attribute__((constructor)) static void test_constructor()
    {
        constructor_was_run = true;
    }

    bool test_crtx()
    {
        print_test_msg("Global constructors");

        if (constructor_was_run)
            print_check_msg(true, "test_constructor was called");
        else
            print_check_msg(false, "test_constructor was not called");

        return constructor_was_run;
    }

} // namespace Kernel::Tests