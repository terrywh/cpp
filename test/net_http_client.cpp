#include <xbond/net/http/client.hpp>
#include <iostream>
using namespace xbond;

int test_net_http_client(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    boost::asio::io_context io;
    
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        net::http::client cli{io};
        boost::beast::http::request<boost::beast::http::empty_body> req {boost::beast::http::verb::get, "/", 11};
        boost::beast::http::response<boost::beast::http::string_body> rsp {};
        boost::system::error_code error;
        cli.execute(net::address{"www.qq.com",80}, req, rsp, ch[error]);
        std::cout << "\t\tbody size: " << rsp.body().size() << std::endl;
    });
    io.run();
}