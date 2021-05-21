#pragma once
#include "../../vendor.h"
#include "detail/client_socket_manager.hpp"
#include "detail/client_execute.hpp"
#include "../address.hpp"
#include "../../coroutine.hpp"

namespace xbond {
namespace net {
namespace http {
// HTTP 客户端，支持简单的连接复用
class client {
 public:
    // 客户端选项
    struct option {
        option();
        std::chrono::steady_clock::duration   timeout; // default 5  seconds
        std::chrono::steady_clock::duration keepalive; // default 50 seconds (if "Connection: Keep-Alive" is present)
    };

 private:
    boost::asio::io_context& io_;
    option option_;
    std::shared_ptr<detail::client_socket_manager> socket_;
    
 public:
    // 构建客户端
    client(boost::asio::io_context& io, option opt = option());
    ~client();
    // 执行请求
    template <class RequestBody, class RequestField, class ResponseBody, class ResponseField, class CompletionToken>
    auto execute(const address& addr, 
        boost::beast::http::request<RequestBody, RequestField>& req,
        boost::beast::http::response<ResponseBody, ResponseField>& rsp,
        CompletionToken&& handler) -> 
            typename boost::asio::async_result<typename std::decay<CompletionToken>::type,
                void(boost::system::error_code)>::return_type {
        
        return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
            detail::client_execute<RequestBody, RequestField, ResponseBody, ResponseField>(
                std::make_shared<detail::client_execute_context<RequestBody, RequestField, ResponseBody, ResponseField>>(
                    socket_, addr, option_.timeout, req, rsp
                )
            ), handler, io_
        );
    }
};

} // namespace http
} // namespace net
} // namespace xbond 
