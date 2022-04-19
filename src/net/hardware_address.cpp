#include <xbond/net/hardware_address.hpp>
#include <iomanip>

namespace xbond {
namespace net {

std::ostream& operator<<(std::ostream& os, const hardware_address& ha) {
    using std::setw;
    os.setf(std::ios_base::hex, std::ios_base::basefield);
    auto f = os.fill('0');
    os  << setw(2) << static_cast<int>(ha.hwaddr_[0]) << ":"
        << setw(2) << static_cast<int>(ha.hwaddr_[1]) << ":"
        << setw(2) << static_cast<int>(ha.hwaddr_[2]) << ":"
        << setw(2) << static_cast<int>(ha.hwaddr_[3]) << ":"
        << setw(2) << static_cast<int>(ha.hwaddr_[4]) << ":"
        << setw(2) << static_cast<int>(ha.hwaddr_[5]);
    os.setf(std::ios_base::dec, std::ios_base::basefield);
    os.fill(f);
    return os;
}

} // namespace net
} // namespace xbond
