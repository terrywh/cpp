#include <xbond/net/server.hpp>
#include <xbond/net/address.hpp>
#include <xbond/net/async_write.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t"

class session {
    boost::asio::ip::tcp::socket socket_;
 public:
    session(boost::asio::ip::tcp::socket&& socket)
    : socket_(std::move(socket)) {}

    void run(coroutine_handler& ch);
};
using server = net::tcp_server<session>;
static std::shared_ptr<server> echo_server;

void session::run(coroutine_handler& ch) {
    boost::system::error_code error;
    std::array<char, 4096> buffer;
    while (true) {
        std::size_t length = socket_.async_read_some(boost::asio::buffer(buffer), ch[error]);
        if (error) break;
        boost::asio::async_write(socket_, boost::asio::buffer(buffer, length), ch[error]);
        if (error) break;
    }
    LOGGER() << "\tsession: close\n";
    echo_server->close();
    LOGGER() << "\tserver:  close\n";
}

class echo_once: public std::enable_shared_from_this<echo_once> {
    boost::asio::ip::tcp::socket socket_;

 public:
    echo_once(boost::asio::io_context& io)
    : socket_(io) {
        
    }
    void start(boost::asio::ip::tcp::endpoint connect) {
        coroutine::start(socket_.get_executor(), [this, connect, self = shared_from_this()] (coroutine_handler& ch) {
            boost::system::error_code error;
            socket_.async_connect(connect, ch[error]);
            if (error) return;
            boost::json::value hello {
                {"hello", "world"},
            };
            net::async_write(socket_, hello, ch[error]);
            if (error) return;
            std::array<char, 4096> rbuffer;
            std::size_t length = socket_.async_read_some(boost::asio::buffer(rbuffer), ch[error]);
            LOGGER() << "\t" << length << " (" << std::string_view{rbuffer.data(), length} << ")\n";
            socket_.close();
            LOGGER() << "\tclient:  close\n";
        });
    }
};

int net_tcp_server_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    echo_server = net::make_server<session>(io, net::address{"127.0.0.1:8888"});
    echo_server->start();
    auto echo_client = std::make_shared<echo_once>(io);
    echo_client->start(net::address{"127.0.0.1:8888"});
    io.run();
    return 0;
}