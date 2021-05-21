#pragma once
#include "../../../vendor.h"
#include "../../address.hpp"
#include "../../detail/socket_connect.hpp"

namespace xbond {
namespace net {
namespace http {

class client;
namespace detail {
// 连接管理器，支持简单的复用机制
class client_socket_manager : public std::enable_shared_from_this<client_socket_manager> {
    boost::asio::io_context&             io_;
    boost::asio::io_context::strand  strand_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::steady_timer          scan_;
    std::chrono::steady_clock::duration ttl_;
    struct cached_socket {
        boost::asio::ip::tcp::socket          socket;
        std::chrono::steady_clock::time_point expire;
    };
    std::multimap<address, cached_socket> cache_;
    // 扫描并清理超过生存周期的缓存连接
    void scan_for_ttl();

 public:
    client_socket_manager(boost::asio::io_context& io, std::chrono::steady_clock::duration ttl);

    boost::asio::io_context& io_context() { return io_; }

    template <class CompleteToken>
    void acquire(const address& addr, boost::beast::tcp_stream& stream, CompleteToken&& handler) {
        boost::asio::post(strand_, [this, addr, &stream, handler = std::move(handler), self = shared_from_this()] () mutable {
            auto i = cache_.find(addr);
            // 找到了还在有效期内的同目标地址的链接
            if (i != cache_.end() && std::chrono::steady_clock::now() < i->second.expire) {
                stream.socket() = std::move(i->second.socket);
                cache_.erase(i);
                handler(boost::system::error_code{});
                return;
            }
            // 建立新连接
            boost::asio::async_compose<CompleteToken, void(boost::system::error_code)>(
                net::detail::socket_connect<boost::asio::ip::tcp>(stream.socket(), addr, resolver_),
                handler, stream.socket(), resolver_
            );
        });
    }
    // 释放链接（保存已备复用）
    // 注意：释放后该 stream 不可用
    template <class CompleteToken>
    void release(const address& addr, boost::beast::tcp_stream& stream, CompleteToken&& handler) {
        boost::asio::post(strand_, [this, addr, &stream, handler = std::move(handler), self = shared_from_this()] () mutable {
            auto now = std::chrono::steady_clock::now();
            cache_.insert({ addr, cached_socket{std::move(stream.socket()), now + ttl_} });
            handler(boost::system::error_code{});
        });
    }

    friend class xbond::net::http::client;
};

} // namespace detail
} // namespace http
} // namespace net 
} // namespace xbond
