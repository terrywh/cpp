#pragma once
#include "address.hpp"
#include "url.hpp"
#include "detail/socket_connect.hpp"
#include "detail/http_proxy_connect.hpp"
#include <boost/asio/compose.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/core/tcp_stream.hpp>

namespace xbond {
namespace net {
// 连接指定地址（支持域名解析）
template <class CompletionToken>
void async_connect(boost::asio::ip::tcp::socket& stream, const net::address& address, boost::asio::ip::tcp::resolver& resolver, CompletionToken&& handler) {
    return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
        detail::socket_connect<boost::asio::ip::tcp>(stream, address, resolver), handler, stream, resolver);
}

template <class CompletionToken>
void async_connect(boost::asio::ip::tcp::socket& stream, boost::asio::streambuf& buffer, const net::url& proxy, const net::address& address, boost::asio::ip::tcp::resolver& resolver, CompletionToken&& handler) {
    if (proxy.scheme == "http") {
        return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
            detail::http_proxy_connect<boost::asio::ip::tcp>(stream, buffer, proxy, address, resolver), handler, stream, resolver);
    }
}


} // namespace net
} // namespace xbond
