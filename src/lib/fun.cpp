#include <fmt/base.h>

namespace toto
{
    int add(int a, int b)
    {
        fmt::print("Adding {} and {}\n", a, b);
        return a + b;
    }

} // namespace toto
