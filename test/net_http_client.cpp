#include <xbond/net/http/client.hpp>
#include <xbond/thread_pool.hpp>
#include <xbond/time/sleep_for.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t\t\t"

int net_http_client_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    std::srand(std::time(nullptr));
    for (int i=0;i<8;++i) {
        coroutine::start(io, [&io, i] (coroutine_handler& ch) {
            time::sleep_for(std::chrono::milliseconds(i * 20), ch);
            net::http::client cli{io};
            for (int j=0;j<50;++j) {
                boost::beast::http::request<boost::beast::http::empty_body> req {boost::beast::http::verb::get, "/", 11};
                req.set(boost::beast::http::field::host, "www.qq.com");
                req.keep_alive(true);
                boost::beast::http::response<boost::beast::http::string_body> rsp {};
                boost::system::error_code error;
                cli.execute(net::address{"www.qq.com",80}, req, rsp, ch[error]);
                std::cout << (rsp.result_int() > 200 ? "." : "x") << std::flush;
                rsp.body().clear();
                time::sleep_for(std::chrono::milliseconds(std::rand()%5), ch);
            }
        });
    }
    thread_pool pool(4, [&io] () {
        io.run();
    });
    pool.wait();
    std::cout << "\n\t\t\tdone" << std::endl;
    return 0;
}