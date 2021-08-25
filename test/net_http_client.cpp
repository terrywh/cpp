#include <xbond/net/http/client.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t\t\t"

int net_http_client_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    
    coroutine::start(io, [&io] (coroutine_handler& ch) {
        net::http::client cli{io};
        boost::beast::http::request<boost::beast::http::empty_body> req {boost::beast::http::verb::get, "/", 11};
        req.set(boost::beast::http::field::host, "www.qq.com");
        req.keep_alive(true);
        boost::beast::http::response<boost::beast::http::string_body> rsp {};
        boost::system::error_code error;
        cli.execute(net::address{"www.qq.com",80}, req, rsp, ch[error]);
        LOGGER() << "\terror: " << error << " status: " << rsp.result() << " body size: " << rsp.body().size() << "\n";
        rsp.body().clear();
        cli.execute(net::address{"www.qq.com",80}, req, rsp, ch[error]);
        LOGGER() << "\terror: " << error << " status: " << rsp.result() << " body size: " << rsp.body().size() << "\n";
        rsp.body().clear();
        cli.execute(net::address{"www.qq.com",80}, req, rsp, ch[error]);
        LOGGER() << "\terror: " << error << " status: " << rsp.result() << " body size: " << rsp.body().size() << "\n";
        rsp.body().clear();
        cli.execute(net::address{"www.qq.com",80}, req, rsp, ch[error]);
        LOGGER() << "\terror: " << error << " status: " << rsp.result() << " body size: " << rsp.body().size() << "\n";
        rsp.body().clear();
    });
    io.run();
}