#include <xbond/net/http/detail/client_socket_manager.hpp>

namespace xbond {
namespace net {
namespace http {
namespace detail {

client_socket_manager::client_socket_manager(boost::asio::io_context& io,
    std::chrono::steady_clock::duration ttl)
: io_(io), strand_(io), resolver_(io), ttl_(ttl), scan_(io) {}

void client_socket_manager::scan_for_ttl() {
    scan_.expires_after(ttl_/2);
    scan_.async_wait([this, self = shared_from_this()] (const boost::system::error_code& error) {
        if (error) return;
        boost::asio::post(strand_, [this, self] () {
            auto now = std::chrono::steady_clock::now();
            for (auto i=cache_.begin(); i!=cache_.end(); ) {
                if (now < i->second.expire)
                    i = cache_.erase(i); // 清理超过有效时间的连接
                else
                    ++i;
            }
        });
        scan_for_ttl();
    });
}

} // namespace detail
} // namespace http
} // namespace net
} // namespace xbond
