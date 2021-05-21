#pragma once
#include "../vendor.h"
#include "detail/address_storage.hpp"

namespace xbond {
namespace net {

// 设备信息
struct device_info {
    int             idx;
    std::string    name;
    detail::hw_address                   hw;
    std::optional<detail::ip_address_v4> v4;
    std::optional<detail::ip_address_v6> v6;
};

} // namespace net
} // namespace xbond
