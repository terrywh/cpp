#include <xbond/coroutine.hpp>
#include <xbond/net/async_connect.hpp>
#include <xbond/time/sleep_for.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t\t"

int net_async_proxy_connect_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        boost::system::error_code error;
        boost::beast::tcp_stream stream(io);
        boost::asio::ip::tcp::resolver resolver(io);
        boost::asio::streambuf buffer;
        net::async_connect(stream.socket(), buffer, net::url { "http://175.27.38.33:23457" }, net::address { "mirrors.tencent.com", 443}, resolver, ch[error]);
        LOGGER() << "async_connect(stream/proxy): (" << error << ") " << error.message() << "\n";

    });
    io.run();
    return 0;
}

int net_async_connect_test(int argc, char* argv[]) {
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
        std::cout << "\n";
    });
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        boost::system::error_code error;
        boost::beast::tcp_stream stream(io);
        boost::asio::ip::tcp::resolver resolver(io);
        net::async_connect(stream.socket(), net::address{"mirrors.tencent.com", 443}, resolver, ch[error]);
        LOGGER() << "async_connect(stream)\terror: " << error << "\n";
    });
    io.run();
    return 0;
}

int net_client_test(int argc, char* argv[]) {
    std::cout << "\t" <<__func__ << "\n";
    net_async_connect_test(argc, argv);
    net_async_proxy_connect_test(argc, argv);
    return 0;
}

