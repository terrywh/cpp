#include <xbond/net/address.hpp>
#include <xbond/utility.hpp>

namespace xbond {
namespace net {

address::address(const struct sockaddr_in& sin) {
    boost::asio::ip::address_v4 addr {sin.sin_addr.s_addr};
    host_ = addr.to_string();
    port_ = sin.sin_port;
    svc_  = std::to_string(port_);
}

address::address(const struct sockaddr_in6& sin6) {
    boost::asio::ip::address_v6 addr { xbond::to_array(sin6.sin6_addr.s6_addr) };
    host_ = addr.to_string();
    port_ = sin6.sin6_port;
    svc_  = std::to_string(port_);
}

} // namespace net
} // namespace xbond
