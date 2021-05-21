#include <xbond/net/client.hpp>
#include <xbond/coroutine.hpp>
using namespace xbond;

#define LOGGER() std::cout << "\t"

int net_client_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        boost::system::error_code error;
        boost::asio::ip::tcp::resolver resolver(io);
        boost::asio::ip::tcp::socket   socket(io);
        net::async_connect(socket, net::address{"www.qq.com", 80}, resolver, ch[error]);
        if (error)
            LOGGER() << "\terror: " << error << "\n"; 
    });
    io.run();

    return 0;
}