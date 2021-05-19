#pragma once
#include "../vendor.h"
#include "../coroutine.hpp"

namespace xbond {
namespace net {

template <class Session>
class tcp_server: public std::enable_shared_from_this<tcp_server<Session>> {
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket     socket_;

    void accept() {
        acceptor_.async_accept(socket_, [this, self = this->shared_from_this()] (const boost::system::error_code& error) {
            if (error) return;
            coroutine::start(socket_.get_executor(), [session = std::make_shared<Session>(std::move(socket_))] (coroutine_handler& ch) {
                session->run(ch);
            });
            accept();
        });
    }
 public:
    tcp_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind)
    : acceptor_(io, bind, true)
    , socket_(io) {
        // boost::asio::ip::tcp::socket::reuse_address reuse_address(true);
        // acceptor_.set_option(reuse_address);
        // acceptor_.bind(bind);
    }
    void start() {
        acceptor_.listen();
        accept();
    }
    void close() {
        acceptor_.close();
    }
};

template <class Session>
std::shared_ptr<tcp_server<Session>> make_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind) {
    return std::make_shared<tcp_server<Session>>(io, bind);
}

} // namespace net
} // namespace xbond
