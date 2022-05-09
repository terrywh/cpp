#include <xbond/net/http/server.hpp>
#include <xbond/net/http/client.hpp>
#include <xbond/net/address.hpp>
#include <xbond/thread_pool.hpp>
#include <xbond/time/sleep_for.hpp>
#include <iostream>
using namespace xbond;
namespace http = boost::beast::http;
namespace asio = boost::asio;

#define LOGGER() std::cout << "\t\t\t"

int net_http_server_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    auto svr = net::http::make_server<boost::beast::flat_static_buffer<4096>>(io, net::address{"127.0.0.1:8888"});
    svr->handle("/world", [] (boost::beast::tcp_stream &stream, boost::beast::flat_static_buffer<4096UL> &buffer, net::http::server_parser& parser, xbond::coroutine_handler &ch) {
        boost::system::error_code& error = ch;
        http::request_parser<http::string_body> req { std::move(parser) };
        http::async_read(stream, buffer, req, ch[error]);
        LOGGER() << "req: " << req.get().body() << std::endl;
        time::sleep_for(std::chrono::milliseconds(5));
        http::response<http::empty_body> rsp {http::status::ok, 11};
        rsp.set(http::field::server, "xbond");
        rsp.chunked(true);
        http::response_serializer<http::empty_body> sr(rsp);
        http::async_write_header(stream, sr, ch[error]);
        for (int i=0;i<10;++i) {
            time::sleep_for(std::chrono::milliseconds(5));
            asio::async_write(stream, http::make_chunk(asio::buffer(std::to_string(i))), ch[error]);
            if (error) goto ON_ERROR;
        }
        asio::async_write(stream, http::make_chunk_last(), ch[error]);
ON_ERROR:
        ;
    });
    svr->handle("/hello", [] (boost::beast::tcp_stream &stream, boost::beast::flat_static_buffer<4096UL> &buffer, net::http::server_parser& parser, xbond::coroutine_handler &ch) {
        boost::system::error_code& error = ch;
        error = {boost::asio::error::fault, boost::system::system_category()};
    });
    coroutine::start(io, [svr] (coroutine_handler ch) {
        svr->run(ch);
    });
    coroutine::start(io, [svr] (coroutine_handler ch) {
        time::sleep_for(std::chrono::seconds(20));
        LOGGER() << "close" << std::endl;
        svr->close();
    });
    coroutine::start(io, [&io, svr] (coroutine_handler& ch) {
        net::http::client cli{io};
        http::request<http::string_body> req {http::verb::get, "/world", 11, "hello"};
        http::response<http::string_body> rsp;
        for (int i=0;i<100;++i) { 
            cli.execute(net::address("127.0.0.1:8888"), req, rsp, ch);
        }
        LOGGER() << "rsp: " << rsp.body() << std::endl;
    });
    thread_pool pool(4, [&io] () {
        io.run();
    });
    pool.wait();
    std::cout << "\n\t\t\tdone" << std::endl;
    return 0;
}