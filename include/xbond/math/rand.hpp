#pragma once
#include <random>
#include <type_traits>

namespace xbond {
namespace math {
namespace rand {
namespace detail {
std::random_device& device();
} // namespace detail

template <class Int, typename = std::is_integral<Int>>
auto integer(Int begin, Int end) -> typename std::decay<Int>::type {
    std::uniform_int_distribution< typename std::decay<Int>::type > dist { begin, end };
    return dist( detail::device() );
}

} // namespace rand
} // namespace math
} // namespace xbond
