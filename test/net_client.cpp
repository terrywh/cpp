#include <xbond/net/client.hpp>
#include <xbond/coroutine.hpp>
#include <xbond/time/sleep_for.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t"

int net_client_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        boost::system::error_code error;
        boost::asio::ip::tcp::resolver resolver(io);
        boost::asio::ip::tcp::socket   socket(io);
        for (int i=0; i<20; ++i) {
            net::async_connect(socket, net::address{"mirrors.tencent.com", 443}, resolver, ch[error]);
            std::cout << "." << std::flush;
            xbond::time::sleep_for(std::chrono::milliseconds(200));
        }
    });
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        boost::system::error_code error;
        boost::beast::tcp_stream stream(io);
        boost::asio::ip::tcp::resolver resolver(io);
        net::async_connect(stream, net::address{"mirrors.tencent.com", 443}, resolver, ch[error]);
        LOGGER() << "async_connect(stream)\terror: " << error << "\n";
    });
    io.run();

    return 0;
}