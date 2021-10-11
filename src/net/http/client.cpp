#include <xbond/net/http/client.hpp>
#include <xbond/time/timer.hpp>
#include <iostream>

namespace xbond {
namespace net {
namespace http {

client::option::option()
:   timeout(std::chrono::seconds(5))
, keepalive(std::chrono::seconds(45)) {}

client::client(boost::asio::io_context& io, client::option option)
: io_(io)
, option_(option)
, manager_(std::make_shared<detail::client_socket_manager>(io, option_.keepalive)) {
    manager_->start();   
}

client::~client() {
    manager_->close();
}
} // namespace http
} // namespace net
} // namespace xbond
