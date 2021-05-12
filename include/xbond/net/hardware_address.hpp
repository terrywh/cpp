#pragma once
#include "../vendor.h"
#include "netlink_route.hpp"

namespace xbond {
namespace net {

class hardware_address {
public:
    using value_type = std::array<std::uint8_t, 6>;
    hardware_address() {
        netlink_route nr;
        nr.foreach_device([this] (const netlink_route::device& device) -> bool {
            if (device.name[0] == 'e') {
                hwaddr_ = device.hw;
                return false;
            }
            return true;
        });
    }
    hardware_address(const value_type& bytes)
    : hwaddr_(bytes) {}
    //
    value_type& bytes() { return hwaddr_; }
    //
    std::string str() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
    const char* data() const { return reinterpret_cast<const char*>(hwaddr_.data()); }
    std::size_t size() const { return hwaddr_.size(); }
private:
    value_type hwaddr_;
    friend std::ostream& operator<<(std::ostream& os, const hardware_address& ha) {
        os.setf(std::ios_base::hex, std::ios_base::basefield);
        auto f = os.fill('0');
        os  << static_cast<int>(ha.hwaddr_[0]) << ":"
            << static_cast<int>(ha.hwaddr_[1]) << ":"
            << static_cast<int>(ha.hwaddr_[2]) << ":"
            << static_cast<int>(ha.hwaddr_[3]) << ":"
            << static_cast<int>(ha.hwaddr_[4]) << ":"
            << static_cast<int>(ha.hwaddr_[5]);
        os.setf(std::ios_base::dec, std::ios_base::basefield);
        os.fill(f);
        return os;
    }
};

} // namespace net
} // namespace xbond

