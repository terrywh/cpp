#pragma once
#include "client_socket_manager.hpp"
#include "../../address.hpp"
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>

namespace xbond {
namespace net {
namespace http {
namespace detail {

template <class Executor, class RequestBody, class ResponseBody>
class client_execute: public boost::asio::coroutine {

 public:
    using context_type = client_execute_context<Executor, RequestBody, ResponseBody>;

 private:
    std::shared_ptr<detail::client_socket_manager> manager_;
    std::shared_ptr<context_type> context_;

 public:
    client_execute(std::shared_ptr<detail::client_socket_manager> mgr, std::shared_ptr<context_type> obj)
    : manager_(mgr)
    , context_(obj) {}

    client_execute(const client_execute& ce) = default;
    // client_execute(client_execute&& ce) = default; // 复制代替移动，防止下述函数调用流程由于 EVALUATION 顺序可能导致的问题

    // AsyncOperation == boost::asio::detail::composed_op< client_execute , void(boost::system::error_code) >
    template <class AsyncOperation>
    inline void operator () (AsyncOperation& self, boost::system::error_code error = {}, std::size_t size = 0) { BOOST_ASIO_CORO_REENTER(this) {
        // 解析域名并获得网络连接
        BOOST_ASIO_CORO_YIELD manager_->acquire(context_, std::move(self));
        if (error) goto DONE;
        // 超时设置
        context_->stream->expires_after(context_->timeout);
        context_->buffer.clear();
        // 发送请求
        context_->request.prepare_payload();
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_write(*context_->stream, context_->request, std::move(self));
        if (error) goto DONE;
        // 接收响应
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_read(*context_->stream, context_->buffer, context_->response, std::move(self));
DONE:
        context_->stream->expires_never();
        context_->error = error == boost::asio::error::operation_aborted ? boost::beast::error::timeout : error;
            
        if (error || context_->response.need_eof()) { // 发生异常（本身可能就是超时）
            BOOST_ASIO_CORO_YIELD manager_->closing(context_, std::move(self));
        } else {
            BOOST_ASIO_CORO_YIELD manager_->release(context_, std::move(self));
        }
        self.complete(context_->error);
    }}
};

} // namespace detail
} // namespace http
} // namespace net
} // namespace xbond
