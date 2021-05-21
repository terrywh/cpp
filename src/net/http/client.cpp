#include <xbond/net/http/client.hpp>

namespace xbond {
namespace net {
namespace http {

client::option::option()
:   timeout(std::chrono::seconds(5))
, keepalive(std::chrono::seconds(50)) {}

client::client(boost::asio::io_context& io, client::option option)
: io_(io)
, option_(option)
, socket_(std::make_shared<detail::client_socket_manager>(io, option_.keepalive)) {
    // boost::asio::post(io_, [s = socket_.get()] {
    //     s->scan_for_ttl();
    // });
}

client::~client() {}

} // namespace http
} // namespace net
} // namespace xbond
