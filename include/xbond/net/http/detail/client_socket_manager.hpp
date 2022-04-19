#pragma once
#include "../../address.hpp"
#include "../../detail/socket_connect.hpp"
#include "../../../time/timer.hpp"
#include <boost/asio/compose.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <map>

namespace xbond {
namespace net {
namespace http {

template <std::size_t BufferSize>
class client;

namespace detail {
// 连接管理器，支持简单的复用机制
class client_socket_manager : public std::enable_shared_from_this<client_socket_manager> {
    boost::asio::io_context&             io_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::ip::tcp::resolver resolver_;

    struct cached_socket {
        std::unique_ptr<boost::beast::tcp_stream> socket;
        std::chrono::steady_clock::time_point     expire;
    };
    std::multimap<address, cached_socket> cache_;
    std::chrono::steady_clock::duration     ttl_;
    std::shared_ptr<boost::asio::steady_timer> ticker_;
    // 清理过期链接
    void sweep(const std::chrono::steady_clock::time_point& now);

 public:
    client_socket_manager(boost::asio::io_context& io, std::chrono::steady_clock::duration ttl);
    boost::asio::io_context& io_context() { return io_; }
    boost::asio::strand<boost::asio::io_context::executor_type>& executor() { return strand_; }

    void start();
    void close();

    template <class CompleteToken>
    void acquire(const address& addr, std::unique_ptr<boost::beast::tcp_stream>& stream, CompleteToken&& handler) {
        boost::asio::post(strand_, [this, addr, &stream, handler = std::move(handler), self = shared_from_this()] () mutable {
            // 找到还在有效期内的同目标地址的链接
            if (auto i = cache_.find(addr); i != cache_.end()) {
                if (std::chrono::steady_clock::now() < i->second.expire) { 
                    stream.swap(i->second.socket);
                    cache_.erase(i);
                    handler(boost::system::error_code{});
                    return;
                }
                cache_.erase(i); // 无效连接移除(然后重新分配)
            }
            // 未能复用，建立新连接
            stream = std::make_unique<boost::beast::tcp_stream>(io_);
            boost::asio::async_compose<CompleteToken, void(boost::system::error_code)>(
                net::detail::stream_connect<boost::asio::ip::tcp>(*stream, addr, resolver_),
                handler, *stream, resolver_, io_
            );
        });
    }
    // 释放链接（保存已备复用）
    // 注意：释放后该 stream 不可用
    template <class CompleteToken>
    void release(const address& addr, std::unique_ptr<boost::beast::tcp_stream>& stream, CompleteToken&& handler) {
        boost::asio::post(strand_, [this, address = addr, stream = std::move(stream), handler = std::move(handler), self = shared_from_this()] () mutable {
            auto now = std::chrono::steady_clock::now();
            cache_.emplace(std::make_pair(address, cached_socket{std::move(stream), now + ttl_}));
            handler(boost::system::error_code{});
        });
    }
    template <std::size_t BufferSize>
    friend class xbond::net::http::client;
};

} // namespace detail
} // namespace http
} // namespace net 
} // namespace xbond
