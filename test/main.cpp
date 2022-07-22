#include <iostream>
#include <boost/asio.hpp>
#include <xbond/net/address.hpp>
#include <xbond/net/http/server.hpp>

using namespace xbond;

extern int core_coroutine_test(int argc, char* argv[]);
extern int core_test(int argc, char* argv[]);
extern int encoding_test(int argc, char* argv[]);
extern int hash_test(int argc, char* argv[]);
extern int net_test(int argc, char* argv[]);
extern int strconv_test(int argc, char* argv[]);
extern int time_test(int argc, char* argv[]);
extern int os_test(int argc, char* argv[]);

// int core_cancel(int argc, char* argv[]) {
//     std::cout << __func__ << ":\n";
//     boost::asio::io_context io;
//     boost::asio::steady_timer tm(io);
//     tm.expires_after(std::chrono::seconds(5000));
//     tm.async_wait([] (const boost::system::error_code& error) {
//         std::cout << "\t" << error << std::endl;
//     });
//     tm.expires_after(std::chrono::seconds(-1));
//     tm.async_wait([] (const boost::system::error_code& error) {
//         std::cout << "\t" << error << std::endl;
//     });
//     io.run();
//     return 0;
// }

int main(int argc, char* argv[]) {
    core_test(argc, argv);
    encoding_test(argc, argv);
    hash_test(argc, argv);
    strconv_test(argc, argv);
    time_test(argc, argv);
    os_test(argc, argv);
    net_test(argc, argv);
    return 0;
}
