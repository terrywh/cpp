#include <xbond/net/http/client.hpp>
#include <xbond/thread_pool.hpp>
#include <xbond/time/sleep_for.hpp>
#include <xbond/math/rand.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout << "\t\t\t"

int net_http_client_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    boost::asio::io_context io;
    net::http::client<>::option opt;
    opt.timeout = std::chrono::milliseconds(1000);
    net::http::client cli{io, opt};
    net::address addr1 {"127.0.0.1",3000};
    net::address addr2 {"www.qq.com",80};
    
    std::srand(std::time(nullptr));


    boost::beast::http::request<boost::beast::http::empty_body> req {boost::beast::http::verb::get, "/", 11};
    req.set(boost::beast::http::field::host, "www.qq.com");
    req.keep_alive(true);

    std::cout << req << std::endl;
    boost::beast::http::response<boost::beast::http::string_body> rsp {};
    boost::system::error_code error;
    cli.execute(addr1, req, rsp, [&rsp] (boost::system::error_code error) {
        std::cout << error << " / " << error.message() << std::endl;
        std::cout << rsp << std::endl;
    });

    for (int i=0;i<64;++i) {
        coroutine::start(io, [&cli, &addr1, &addr2, i] (coroutine_handler& ch) {
            for (int j=0;j<1000;++j) {
                time::sleep_for(std::chrono::milliseconds(math::rand::integer(0, 50)), ch);
                boost::beast::http::request<boost::beast::http::empty_body> req {boost::beast::http::verb::get, "/", 11};
                req.set(boost::beast::http::field::host, "www.qq.com");
                req.keep_alive(true);
                boost::beast::http::response<boost::beast::http::string_body> rsp {};
                boost::system::error_code error;
                cli.execute(addr1, req, rsp, ch[error]);
                rsp.body().clear();
            }
        });
    }
    thread_pool pool(2, [&io] () {
        io.run();
    });
    pool.wait();
    std::cout << "\n\t\t\tdone" << std::endl;
    return 0;
}