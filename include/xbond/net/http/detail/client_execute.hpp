#pragma once
#include "client_socket_manager.hpp"
#include "../../address.hpp"
#include <boost/beast/core/flat_static_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>

namespace xbond {
namespace net {
namespace http {
namespace detail {

template <class RequestBody, class RequestField, class ResponseBody, class ResponseField, std::size_t BufferSize = 16 * 1024>
struct client_execute_context {
    net::address address;
    std::chrono::steady_clock::duration timeout;
    boost::beast::http::request<RequestBody, RequestField>&    request;
    boost::beast::http::response<ResponseBody, ResponseField>& response;
    boost::beast::flat_static_buffer<BufferSize> buffer;
    std::unique_ptr<boost::beast::tcp_stream> stream;

    client_execute_context(boost::asio::io_context& io, net::address addr,
        std::chrono::steady_clock::duration to,
        boost::beast::http::request<RequestBody, RequestField>& req,
        boost::beast::http::response<ResponseBody, ResponseField>& rsp)
    : address(addr)
    , timeout(to)
    , request(req), response(rsp) {}

};
template <class RequestBody, class RequestField, class ResponseBody, class ResponseField>
class client_execute: public boost::asio::coroutine {

 public:
    using context_type = client_execute_context<RequestBody, RequestField, ResponseBody, ResponseField>;

 private:
    std::shared_ptr<detail::client_socket_manager> manager_;
    std::shared_ptr<context_type> context_;

 public:
    client_execute(std::shared_ptr<detail::client_socket_manager> mgr, std::shared_ptr<context_type> obj)
    : manager_(mgr)
    , context_(obj) {}

    // AsyncOperation == boost::asio::detail::composed_op< client_execute , void(boost::system::error_code) >
    template <class AsyncOperation>
    inline void operator () (AsyncOperation& self, boost::system::error_code error = {}, std::size_t size = 0) { BOOST_ASIO_CORO_REENTER(this) {
        // 解析域名并获得网络连接
        BOOST_ASIO_CORO_YIELD manager_->acquire(context_->address, context_->stream, std::move(self));
        if (error) return self.complete(error);
        // 超时设置
        context_->stream->expires_after(context_->timeout);
        context_->buffer.clear();
        // 发送请求
        context_->request.prepare_payload();
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_write(*context_->stream, context_->request, std::move(self));
        if (error) return self.complete(error);
        // 接收响应
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_read(*context_->stream, context_->buffer, context_->response, std::move(self));
        if (error) return self.complete(error);
        // 执行完毕（结束超时计时）
        context_->stream->expires_never();
        // 连接回收复用
        if (context_->response.need_eof()) context_->stream->close();
        else BOOST_ASIO_CORO_YIELD manager_->release(context_->address, context_->stream, std::move(self));
        // 成功响应回调
        self.complete({});
    } }
};

} // namespace detail
} // namespace http
} // namespace net
} // namespace xbond
