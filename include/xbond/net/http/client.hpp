#pragma once
#include "detail/client_socket_manager.hpp"
#include "detail/client_execute.hpp"
#include "../address.hpp"
#include "../../coroutine.hpp"
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/empty_body.hpp>

namespace xbond {
namespace net {
namespace http {
// HTTP 客户端，支持简单的连接复用
template <std::size_t BufferSize = 16 * 1024>
class client {
 public:
    // 客户端选项
    struct option {
        std::chrono::steady_clock::duration   timeout = std::chrono::seconds(5);  // default 5  seconds
        std::chrono::steady_clock::duration keepalive = std::chrono::seconds(50); // default 50 seconds (if "Connection: Keep-Alive" is present)
        std::uint64_t header_limit = 8 * 1024ul;        // 响应头限制
        std::uint64_t body_limit   = 8 * 1024 * 1024ul; // 响应体限制
    };

 private:
    boost::asio::io_context& io_;
    option option_;
    std::shared_ptr<detail::client_socket_manager> manager_;
 public:
    // 构建客户端
    client(boost::asio::io_context& io, const option& opt = {})
    : io_(io)
    , option_(opt)
    , manager_(std::make_shared<detail::client_socket_manager>(io, option_.keepalive)) {
        manager_->start();   
    }
    ~client() {
        manager_->close();
    }
    // 执行请求
    template <class RequestBody, class ResponseBody, class ExecuteHandler>
    void execute(const address& addr, boost::beast::http::request<RequestBody>& req,
        boost::beast::http::response<ResponseBody>& rsp, ExecuteHandler&& handler) {
        
        boost::beast::http::response_parser<ResponseBody> parser { std::move(rsp) };
        parser.header_limit(option_.header_limit);
        parser.body_limit(option_.body_limit);
        
        execute(addr, req, parser, std::forward<ExecuteHandler>(handler));
        rsp = parser.release();
    }

    template <class RequestBody, class ResponseBody, class ExecuteHandler>
    void execute(const address& addr, boost::beast::http::request<RequestBody>& req,
        boost::beast::http::response_parser<ResponseBody>& rsp, ExecuteHandler&& handler) {
        boost::asio::async_compose<ExecuteHandler, void(boost::system::error_code)>(
            detail::client_execute<RequestBody, ResponseBody>(
                manager_,
                std::make_shared<detail::client_execute_context<RequestBody, ResponseBody, BufferSize>>(
                    io_, addr, option_.timeout, req, rsp
                )
            ), handler, io_
        );
    }
};

} // namespace http
} // namespace net
} // namespace xbond 
