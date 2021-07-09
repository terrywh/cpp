#pragma once
#include "../../../vendor.h"
#include "client_socket_manager.hpp"
#include "../../address.hpp"

namespace xbond {
namespace net {
namespace http {
namespace detail {

template <class RequestBody, class RequestField, class ResponseBody, class ResponseField>
class client_execute_context: public boost::asio::coroutine {
    std::shared_ptr<client_socket_manager> manager_;
    net::address address_;
    std::chrono::steady_clock::duration timeout_;
    boost::beast::http::request<RequestBody, RequestField>&    req_;
    boost::beast::http::response<ResponseBody, ResponseField>& rsp_;
    boost::beast::tcp_stream  stream_;
    boost::beast::flat_buffer buffer_;

 public:
    client_execute_context(std::shared_ptr<client_socket_manager> mgr, net::address addr,
        std::chrono::steady_clock::duration to,
        boost::beast::http::request<RequestBody, RequestField>& req,
        boost::beast::http::response<ResponseBody, ResponseField>& rsp)
    : manager_(mgr)
    , address_(addr)
    , timeout_(to)
    , req_(req), rsp_(rsp)
    , stream_(manager_->io_context()) {}

    template <class AsyncOperation>
    void operator()(AsyncOperation& self, boost::system::error_code error, std::size_t size = 0) { BOOST_ASIO_CORO_REENTER(this) {
        // 解析域名并获得网络连接
        BOOST_ASIO_CORO_YIELD manager_->acquire(address_, stream_, std::move(self));
        if (error) return self.complete(error);
        // 超时设置
        stream_.expires_after(timeout_);
        buffer_.clear();
        // 发送请求
        req_.prepare_payload();
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_write(stream_, req_, std::move(self));
        if (error) return self.complete(error);
        // 接收响应
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_read(stream_, buffer_, rsp_, std::move(self));
        if (error) return self.complete(error);
        // 执行完毕（结束超时计时）
        stream_.expires_never();
        // 连接回收复用
        if (rsp_.need_eof()) stream_.close();
        else BOOST_ASIO_CORO_YIELD manager_->release(address_, stream_, std::move(self));
        // 成功响应回调
        self.complete({});
    }}
    // 
};
template <class RequestBody, class RequestField, class ResponseBody, class ResponseField>
class client_execute: public boost::asio::coroutine {

 public:
    using context_type = client_execute_context<RequestBody, RequestField, ResponseBody, ResponseField>;

 private:
    std::shared_ptr<context_type> context_;

 public:
    client_execute(std::shared_ptr<context_type> obj)
    : context_(obj) {}

    // AsyncOperation == boost::asio::detail::composed_op< client_execute , void(boost::system::error_code) >
    template <class AsyncOperation>
    inline void operator () (AsyncOperation& self, boost::system::error_code error = {}, std::size_t size = 0) {
        (*context_)(self, error);
    }
};

} // namespace detail
} // namespace http
} // namespace net
} // namespace xbond
