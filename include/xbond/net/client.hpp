#pragma once
#include "address.hpp"
#include "detail/socket_connect.hpp"

namespace xbond {
namespace net {
// 连接指定地址（支持域名解析）
template <class CompletionToken>
void async_connect(boost::asio::ip::tcp::socket& socket, const net::address& address, boost::asio::ip::tcp::resolver& resolver, CompletionToken&& handler) {
    return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
        detail::socket_connect<boost::asio::ip::tcp>(socket, address, resolver), handler, socket, resolver);
}
// 连接指定地址（支持域名解析）
template <class CompletionToken>
void async_connect(boost::asio::ip::udp::socket& socket, const net::address& address, boost::asio::ip::udp::resolver& resolver, CompletionToken&& handler) {
    return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
        detail::socket_connect<boost::asio::ip::udp>(socket, address, resolver), handler, socket, resolver);
}

} // namespace net
} // namespace xbond
