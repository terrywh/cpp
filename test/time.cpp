#include <xbond/time/date.hpp>
#include <xbond/time/delta_clock.hpp>
#include <xbond/time/timer.hpp>
#include <iostream>
using namespace xbond;

int time_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    auto tp = std::chrono::system_clock::now();
    auto dp = time::floor<time::days>(tp);
    auto date = time::year_month_day{dp};
    auto time = time::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp));
    std::cout << "\t" << date << " " << time << "\n";
    std::cout << "\t" << time::iso(time::delta_clock::get()) << "\n";
    boost::asio::io_context io;
    auto ticker = time::tick(io, std::chrono::seconds(1), [] () {
        std::cout << "." << std::flush;
    });
    time::after(io, std::chrono::milliseconds(3500), [ticker] () {
        std::cout << "\tstop ticking\n";
        ticker->close();
    });
    io.run();
    std::cout << "\t" << "done\n";
    return 0;
}