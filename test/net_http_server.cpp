#include <xbond/net/http/server.hpp>
#include <xbond/net/address.hpp>
#include <xbond/thread_pool.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t\t\t"

using http_request  = boost::beast::http::request<boost::beast::http::string_body>;
using http_response = boost::beast::http::response<boost::beast::http::string_body>;
class request_handler {
public:
    request_handler(boost::beast::tcp_stream& stream) {}
    void operator()(http_request& req, http_response& rsp, coroutine_handler& ch) {
        rsp.result(boost::beast::http::status::ok);
        rsp.body() = req.body();
    }
};
using http_server   = net::http::server<boost::beast::http::string_body>;

int net_http_server_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    auto svr = net::http::make_server<boost::beast::http::string_body>(io, net::address{"127.0.0.1:8888"});
    coroutine::start(io, [svr] (coroutine_handler ch) {
        svr->run<request_handler>(ch);
    });
    thread_pool pool(4, [&io] () {
        io.run();
    });
    pool.wait();
    std::cout << "\n\t\t\tdone" << std::endl;
    return 0;
}