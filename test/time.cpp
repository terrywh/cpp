#include <xbond/time/date.hpp>
#include <xbond/time/delta_clock.hpp>
#include <xbond/time/date.hpp>
#include <xbond/time/timer.hpp>
#include <xbond/time/sleep_for.hpp>
#include <xbond/thread_pool.hpp>
#include <xbond/math/rand.hpp>
#include <iostream>
using namespace xbond;

int tzone_test(int argc, char* argv[]) {
    std::cout << "\t" << __func__ << "\n";
    using namespace xbond::time;
    auto now = make_zoned(current_zone(), std::chrono::system_clock::now());
    std::cout << "\t\toffset: " << now.get_info().offset << "\n";
    auto now_time = make_time(now.get_local_time() - time::floor<time::days>(now.get_local_time()));
    auto dst = make_zoned(current_zone(), time::floor<time::days>(now.get_sys_time()) + 
        std::chrono::hours(1) + std::chrono::seconds(math::rand::integer(0, 5400)) - now.get_info().offset);
    auto dst_time = make_time(dst.get_local_time() - time::floor<time::days>(dst.get_local_time()));
    std::cout << "\t\t" << now << " ~ " << dst << "\n";
    std::cout << "\t\t" << now.get_sys_time() << " ~ " << dst.get_sys_time() << "\n";
    std::cout << "\t\t" << now_time.hours() << " ~ " << dst_time.hours() << "\n";
    return 0;
}

int clock_test(int argc, char* argv[]) {
    std::cout << "\t" << __func__ << "\n";
    time::delta_clock x;
    std::cout << "\t\t" << static_cast<std::int64_t>(x) << "\n";
    // auto tp = std::chrono::system_clock::now();
    auto tp = static_cast<std::chrono::system_clock::time_point>(x);
    auto dp = time::floor<time::days>(tp);
    auto date = time::year_month_day{dp};
    auto time = time::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp));
    std::cout << "\t\t" << date << " " << time << "\n";
    std::cout << "\t\t" << time::iso(time::delta_clock::get()) << "\n";
    return 0;
}

int timer_test(int argc, char* argv[]) {
    std::cout << "\t" << __func__ << "\n";
    boost::asio::io_context io;
    
    time::tick(io, std::chrono::seconds(2), [] (boost::system::error_code error) {
        std::cout << "\t\ttick\n";
    });
    time::after(io, std::chrono::seconds(10), [&io] (boost::system::error_code error) {
        std::cout << "\t\tstop\n";
        io.stop();
    });
    coroutine::start(io, [] (xbond::coroutine_handler& ch) {
        time::sleep_for(std::chrono::seconds(5));
        std::cout << "\t\tdone\n";
    });
    thread_pool pool { 4, [] (boost::asio::io_context& io) {
        io.run();
    }, std::ref(io) };
    return 0;
}

int time_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    tzone_test(argc, argv);
    clock_test(argc, argv);
    timer_test(argc, argv);
    return 0;
}