#pragma once
#include "../../address.hpp"
#include "../../detail/socket_connect.hpp"
#include "../../../time/date.hpp"
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
template <class Executor, class RequestBody, class ResponseBody, std::size_t BufferSize = 16 * 1024>
struct client_execute_context {
    Executor& executor;
    net::address                                        address;
    std::chrono::steady_clock::duration                 timeout;
    boost::beast::http::request<RequestBody>&           request;
    boost::beast::http::response<ResponseBody>&        response;
    boost::beast::flat_static_buffer<BufferSize>         buffer;
    std::unique_ptr<boost::beast::tcp_stream>            stream;
    boost::system::error_code                             error;

    client_execute_context(Executor& e, net::address addr,
        std::chrono::steady_clock::duration to,
        boost::beast::http::request<RequestBody>& req,
        boost::beast::http::response<ResponseBody>& rsp)
    : executor(e)
    , address(addr), timeout(to)
    , request(req), response(rsp) {
        
    }

    // ~client_execute_context() {
    //     std::cout << "X:" << this << std::endl;
    // }

    void dump(const char* prefix) {
        boost::system::error_code error;
        std::stringstream ss1, ss2, ss3;
        ss1 << address;
        ss2 << stream->socket().remote_endpoint(error);
        std::string ca = ss1.str(), cs = ss2.str();

        ss3 << time::iso(std::chrono::system_clock::now()) 
            << " <" << prefix << "> "
            << this
            << " context->address = " << ca
            << " context->stream = " << cs
            << " equal = " << (ca == cs)
            << std::endl;
        
        std::cout << ss3.str();
    }
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

    template <class Executor, class RequestBody, class ResponseBody, std::size_t BufferSize, class AcquireHandler>
    void acquire(std::shared_ptr<client_execute_context<Executor, RequestBody, ResponseBody, BufferSize>> context, AcquireHandler&& handler) {
        boost::asio::post(strand_, [this, context, handler = std::move(handler), self = shared_from_this()] () mutable {
            // 找到还在有效期内的同目标地址的链接
            auto r = cache_.equal_range(context->address);
            for (auto i = r.first; i != r.second; ) {
                if (std::chrono::steady_clock::now() < i->second.expire) {
                    context->stream = std::make_unique<boost::beast::tcp_stream>( std::move(*i->second.socket) );
                    cache_.erase(i);
                    context->dump("revisit");
                    boost::asio::post(context->executor, std::move(handler));
                    return;
                }
                i = cache_.erase(i); // 无效连接移除(然后重新分配)
            }
            // 建立新的连接(注意回调回到 context->executor 上下文)
            auto on_connect = [context, handler = std::move(handler)] (boost::system::error_code error) mutable {
                context->dump("connect");
                boost::asio::post(context->executor, [context, handler = std::move(handler), error] () mutable {
                    handler(error);
                });
            };
            context->stream = std::make_unique<boost::beast::tcp_stream>(io_);
            boost::asio::async_compose<decltype(on_connect), void(boost::system::error_code)>(
                net::detail::socket_connect<boost::asio::ip::tcp>(context->stream->socket(), context->address, resolver_),
                on_connect, *context->stream, resolver_, context->executor
            );
        });
    }
    // 释放链接（保存已备复用）
    // 注意：释放后该 stream 不可用
    template <class Executor, class RequestBody, class ResponseBody, std::size_t BufferSize, class ReleaseHandler>
    void release(std::shared_ptr<client_execute_context<Executor, RequestBody, ResponseBody, BufferSize>> context, ReleaseHandler&& handler) {
        boost::asio::post(strand_, [this, context, handler = std::move(handler), self = shared_from_this()] () mutable {
            auto now = std::chrono::steady_clock::now();
            context->dump("release");
            cache_.emplace(std::make_pair(context->address, cached_socket{
                std::make_unique<boost::asio::ip::tcp::socket>(std::move(context->stream->release_socket())), now + ttl_}));
            boost::asio::post(context->executor, std::move(handler));
        });
    }
    // 延迟关闭
    template <class Executor, class RequestBody, class ResponseBody, std::size_t BufferSize, class ClosingHandler>
    void closing(std::shared_ptr<client_execute_context<Executor, RequestBody, ResponseBody, BufferSize>> context, ClosingHandler&& handler) {
        boost::asio::post(strand_, [this, context, handler = std::move(handler), self = shared_from_this()] () mutable {
            auto now = std::chrono::steady_clock::now();
            cache_.emplace(std::make_pair(context->address, cached_socket{
                std::make_unique<boost::asio::ip::tcp::socket>(std::move(context->stream->release_socket())), now}));
            boost::asio::post(context->executor, std::move(handler));
        });
    }

    template <std::size_t BufferSize>
    friend class xbond::net::http::client;
};

} // namespace detail
} // namespace http
} // namespace net 
} // namespace xbond
