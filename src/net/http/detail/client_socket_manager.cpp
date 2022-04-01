#include <xbond/net/http/detail/client_socket_manager.hpp>
#include <xbond/reference_pointer.hpp>

namespace xbond {
namespace net {
namespace http {
namespace detail {

client_socket_manager::client_socket_manager(boost::asio::io_context& io, std::chrono::steady_clock::duration ttl)
: io_(io), strand_(boost::asio::make_strand(io)), resolver_(io), ttl_(ttl) {
    ticker_ = std::make_shared<boost::asio::steady_timer>(strand_);
}

void client_socket_manager::start() {
    ticker_->expires_after(ttl_ / 5);
    ticker_->async_wait([this, self = shared_from_this()] (const boost::system::error_code& error) {
        if (error) return;
        sweep(std::chrono::steady_clock::now());
        start();
    });
}

void client_socket_manager::close() {
    assert(ticker_);
    ticker_->cancel();
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
