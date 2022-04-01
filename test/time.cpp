#include <xbond/time/date.hpp>
#include <xbond/time/delta_clock.hpp>
#include <xbond/time/timer.hpp>
#include <iostream>
using namespace xbond;

int time_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    time::delta_clock x;
    std::cout << "\t" << static_cast<std::int64_t>(x) << "\n";
    // auto tp = std::chrono::system_clock::now();
    auto tp = static_cast<std::chrono::system_clock::time_point>(x);
    auto dp = time::floor<time::days>(tp);
    auto date = time::year_month_day{dp};
    auto time = time::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp));
    std::cout << "\t" << date << " " << time << "\n";
    std::cout << "\t" << time::iso(time::delta_clock::get()) << "\n";
    boost::asio::io_context io;
    
    auto timer = time::tick(io, std::chrono::seconds(3), [] () {
        std::cout << "\t tick\n";
    });
    time::after(io, std::chrono::seconds(10), [timer] () {
        std::cout << "\t" << "stop ticker:\n";
        timer->close();
    });
    io.run();
    return 0;
}