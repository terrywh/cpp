#pragma once
#include "../../vendor.h"
#include "../address.hpp"
#include "../../coroutine.hpp"

namespace xbond {
namespace net {
namespace http {

// TODO(terryhaowu): 支持并行客户端
// TODO(terryhaowu): 减少复制
class client {
    boost::beast::tcp_stream stream_;
    boost::asio::ip::tcp::resolver resolver_;
    address  address_;
    boost::beast::flat_buffer buffer_;
    std::chrono::steady_clock::duration timeout_;
public:
    client(boost::asio::io_context& io)
    : stream_(io), resolver_(io), timeout_(std::chrono::seconds(10)) {}

    ~client() {
        
    }

    void timeout(std::chrono::steady_clock::duration timeout) {
        timeout_ = timeout;
    }

    template <class ReqBody, class ReqFields, class RspBody, class RspFields>
    void execute(const address& addr, 
        boost::beast::http::request<ReqBody, ReqFields>& req,
        boost::beast::http::response<RspBody, RspFields>& rsp,
        coroutine_handler& ch) {
        
        boost::system::error_code error, &ec = ch;
        if (address_ != addr) {
            if (stream_.socket().is_open()) stream_.close();
            address_ = addr;
        }
        // 超时计时开始
        stream_.expires_after(timeout_);
        if (!stream_.socket().is_open()) {
            // 解析域名
            boost::asio::ip::tcp::resolver::results_type ep;
            resolver_.async_resolve(address_.host(), address_.service(),
                [&ch, &ep] (const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type rs) {
                
                ep = rs;
                ch.resume(error);
            });
            ch.yield(error);
            if (error) {
                ec = error;
                return;
            }
            // 建立连接
            boost::beast::get_lowest_layer(stream_).async_connect(ep, 
                [&ch] (const boost::system::error_code& error, boost::asio::ip::tcp::resolver::endpoint_type ep) {
                ch.resume(error);
            });
            ch.yield(error);
            if (error) {
                ec = error;
                return;
            }
            boost::asio::ip::tcp::socket::keep_alive keep_alive(true);
            boost::beast::get_lowest_layer(stream_).socket().set_option(keep_alive);
        }
        buffer_.clear();
        // 发送请求
        req.prepare_payload();
        boost::beast::http::async_write(stream_, req, ch[error]);
        if (error) {
            // TODO(terryhaowu): 复用的连接可能已失效，需要处理 EPIPE 问题 重试
            ec = error;
            return;
        }
        // 接收响应
         boost::beast::http::async_read(stream_, buffer_, rsp, ch[error]);
        if (error) {
            // TODO(terryhaowu): 处理 error == boost::asio::error::operation_aborted 停止
            // TODO(terryhaowu): 处理 boost::asio::error::eof / http::error::end_of_stream 重试
            ec = error;
            return;
        }
        stream_.expires_never();
        if (rsp.need_eof()) stream_.close();
    }
};

} // namespace http
} // namespace net
} // namespace xbond 
