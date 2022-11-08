#pragma once
#include "../../address.hpp"
#include "../../detail/socket_connect.hpp"
#include "../../../time/timer.hpp"
#include <boost/asio/compose.hpp>
#include <boost/beast/core/flat_static_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <map>

namespace xbond {
namespace net {
namespace http {

template <std::size_t BufferSize>
class client;

namespace detail {
// 请求执行上下文
template <class RequestBody, class ResponseBody, std::size_t BufferSize = 16 * 1024>
struct client_execute_context {
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    net::address                                        address;
    std::chrono::steady_clock::duration                 timeout;
    boost::beast::http::request<RequestBody>&           request;
    boost::beast::http::response_parser<ResponseBody>& response;
    boost::beast::flat_static_buffer<BufferSize>         buffer;
    std::unique_ptr<boost::beast::tcp_stream>            stream;

    client_execute_context(boost::asio::io_context& io, net::address addr,
        std::chrono::steady_clock::duration to,
        boost::beast::http::request<RequestBody>& req,
        boost::beast::http::response_parser<ResponseBody>& rsp)
    : strand(boost::asio::make_strand(io))
    , address(addr), timeout(to)
    , request(req), response(rsp) {
        
    }
<<<<<<< HEAD
=======

    ~client_execute_context() {
        // if (stream) std::cout << "x";
    }
>>>>>>> dda887c (尝试修复: 可能的超时引起的重复关闭问题;)
};
// 连接管理器，支持简单的复用机制
class client_socket_manager : public std::enable_shared_from_this<client_socket_manager> {
    boost::asio::io_context&             io_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::ip::tcp::resolver resolver_;

    struct cached_socket {
        std::unique_ptr<boost::asio::ip::tcp::socket> socket;
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

    template <class RequestBody, class ResponseBody, std::size_t BufferSize, class AcquireHandler>
    void acquire(std::shared_ptr<client_execute_context<RequestBody, ResponseBody, BufferSize>> context, AcquireHandler&& handler) {
        boost::asio::post(strand_, [this, context, handler = std::move(handler), self = shared_from_this()] () mutable {
            // 找到还在有效期内的同目标地址的链接
            if (auto i = cache_.find(context->address); i != cache_.end()) {
                if (std::chrono::steady_clock::now() < i->second.expire) {
                    context->stream = std::make_unique<boost::beast::tcp_stream>( std::move(*i->second.socket) );
                    cache_.erase(i);
                    boost::asio::post(context->strand, std::move(handler));
                    return;
                }
                cache_.erase(i); // 无效连接移除(然后重新分配)
            }
            // 建立新的连接：使用 context::strand 保持其超时
            context->stream = std::make_unique<boost::beast::tcp_stream>(context->strand);
            boost::asio::async_compose<AcquireHandler, void(boost::system::error_code)>(
                net::detail::stream_connect<boost::asio::ip::tcp>(*context->stream, context->address, resolver_),
                handler, *context->stream, resolver_, context->strand
            );
        });
    }
    // 释放链接（保存已备复用）
    // 注意：释放后该 stream 不可用
    template <class RequestBody, class ResponseBody, std::size_t BufferSize, class AcquireHandler>
    void release(std::shared_ptr<client_execute_context<RequestBody, ResponseBody, BufferSize>> context, AcquireHandler&& handler) {
        boost::asio::post(strand_, [this, context, handler = std::move(handler), self = shared_from_this()] () mutable {
            auto now = std::chrono::steady_clock::now();
            cache_.emplace(std::make_pair(context->address, cached_socket{
                std::make_unique<boost::asio::ip::tcp::socket>(std::move(context->stream->release_socket())), now + ttl_}));
            boost::asio::post(context->strand, std::move(handler));
        });
    }
    template <std::size_t BufferSize>
    friend class xbond::net::http::client;
};

} // namespace detail
} // namespace http
} // namespace net 
} // namespace xbond
