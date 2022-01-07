#include <xbond/net/hardware_address.hpp>

namespace xbond {
namespace net {

std::ostream& operator<<(std::ostream& os, const hardware_address& ha) {
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

} // namespace net
} // namespace xbond
