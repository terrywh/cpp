#include <xbond/net/http/detail/client_socket_manager.hpp>
#include <xbond/reference_pointer.hpp>

namespace xbond {
namespace net {
namespace http {
namespace detail {

client_socket_manager::client_socket_manager(boost::asio::io_context& io, std::chrono::steady_clock::duration ttl)
: io_(io), strand_(boost::asio::make_strand(io)), resolver_(io), ttl_(ttl) {
    
}

void client_socket_manager::start() {
    ticker_ = xbond::time::tick(strand_, ttl_ / 5, [this, self = shared_from_this()] () {
        sweep(std::chrono::steady_clock::now());
    });
}

void client_socket_manager::close() {
    assert(ticker_);
    ticker_->close();
}

void client_socket_manager::sweep(const std::chrono::steady_clock::time_point& now) {
    for (auto i=cache_.begin(); i!=cache_.end(); ) {
        if (now < i->second.expire)
            i = cache_.erase(i); // 清理超过有效时间的连接
        else
            ++i;
    }
}

} // namespace detail
} // namespace http
} // namespace net
} // namespace xbond
