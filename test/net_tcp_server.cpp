#include <xbond/net/server.hpp>
using namespace xbond;

class session;

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
    std::cout << "\t\tsession: close\n";
    echo_server->close();
    std::cout << "\t\tserver:  close\n";
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
            std::array<char, 4096> sbuffer;
            std::memcpy(sbuffer.data(), "hello", 5);
            boost::asio::async_write(socket_, boost::asio::buffer(sbuffer, 5), ch[error]);
            if (error) return;
            std::array<char, 4096> rbuffer;
            std::size_t length = socket_.async_read_some(boost::asio::buffer(rbuffer), ch[error]);
            std::cout << length << "(" << std::string_view{rbuffer.data(), length} << ")\n";
            socket_.close();
            std::cout << "\t\tclient:  close\n";
        });
    }
};

int net_tcp_server_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n\t\t";
    boost::asio::io_context io;
    echo_server = net::make_server<session>(io, {boost::asio::ip::make_address("127.0.0.1"), 8888});
    echo_server->start();
    auto echo_client = std::make_shared<echo_once>(io);
    echo_client->start({boost::asio::ip::make_address("127.0.0.1"), 8888});
    io.run();
}