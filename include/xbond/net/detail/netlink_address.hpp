#pragma once
#include <array>

namespace xbond {
namespace net {
    using    hw_address = std::array<std::uint8_t,  6>;
    using ip_address_v4 = std::array<std::uint8_t,  4>;
    using ip_address_v6 = std::array<std::uint8_t, 16>;
} // namespace net
} // namespace xbond
