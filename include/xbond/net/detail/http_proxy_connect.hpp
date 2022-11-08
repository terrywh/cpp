#pragma once
#include "../url.hpp"
#include "../address.hpp"
#include "../../encoding/base64.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>

namespace xbond {
namespace net {
namespace detail {


template <class Protocol>
class http_proxy_connect {
    boost::asio::streambuf&        buffer_;
    const net::url&                 proxy_;
    const net::address&           address_;
    typename Protocol::resolver& resolver_;
    typename Protocol::socket&     socket_;
    enum class status {
        connecting,
        http_requesting,
        http_responding,
        done,
    } status_;

 public:
    http_proxy_connect(typename Protocol::socket& stream, boost::asio::streambuf& buffer, const net::url& proxy, const net::address& address, typename Protocol::resolver& resolver)
    : buffer_(buffer), proxy_(proxy), address_(address), resolver_(resolver), socket_(stream), status_(status::connecting) {}

    template <class AsyncOperation>
    void operator()(AsyncOperation& self, boost::system::error_code error = {}, std::size_t size = 0) {
        if (error) return self.complete(error);

        switch(status_) {
        case status::connecting: 
            status_ = status::http_requesting;
            resolver_.async_resolve(proxy_.domain, proxy_.svc(), std::move(self));
            break;
        case status::http_requesting:
            status_ = status::http_responding;
            write_http_req(self);
            break;
        case status::http_responding:
            status_ = status::done;
            read_http_rsp(self);
            break;
        case status::done:
            return self.complete(error);
        }
    }

    template <class AsyncOperation>
    void operator()(AsyncOperation& self, boost::system::error_code error, typename Protocol::resolver::results_type rt) {
        if (error) return self.complete(error);
        boost::asio::async_connect(socket_, rt, std::move(self));
    }

    template <class AsyncOperation>
    void operator()(AsyncOperation& self, boost::system::error_code error, typename Protocol::endpoint ep) {
        if (error || proxy_.empty()) return self.complete(error);
        this->operator()(self); // 确认经由代理 HTTP / SOCKS5 或直连接口
    }

    template <class AsyncOperation>
    void write_http_req(AsyncOperation& self) {
        std::ostream req (&buffer_);
        std::stringstream auth;
        auth << proxy_.user << ":" << proxy_.password;
        
        req << "CONNECT " << address_.str() << " HTTP/1.1\r\n"
            << "Host: " << address_.str() << "\r\n"
            << "Proxy-Connection: Keep-Alive\r\n"
            << "Proxy-Authorization: Basic " << encoding::base64::encode(auth.str()) << "\r\n"
            << "\r\n";

        boost::asio::async_write(socket_, buffer_.data(), std::move(self));
    }

    template <class AsyncOperation>
    void read_http_rsp(AsyncOperation& self) {
        buffer_.consume(buffer_.size());
        auto rsp = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>();
        boost::beast::http::async_read(socket_, buffer_, *rsp,
            [rsp, self = std::move(self)] (const boost::system::error_code& error, std::size_t size) mutable {
            if (rsp->result() != boost::beast::http::status::ok) self(
                boost::system::error_code{
                    boost::beast::errc::connection_refused,
                    boost::beast::system_category()
                }, size);
            else self(error, size);
        });
        // boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(buffer_), boost::asio::string_view{"\r\n\r\n", 4}, std::move(self));
    }
};

} // namespace detail
} // namespace net
} // namespace xbond
