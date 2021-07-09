#include <xbond/time/date.hpp>
#include <xbond/container_sink.hpp>
using namespace xbond;

namespace xbond {
namespace time {

std::string iso(std::chrono::system_clock::time_point time_point, std::chrono::hours zone_offset) {
    using sink = container_sink<std::string>;

    time_point += zone_offset;
    auto date_point = time::floor<time::days>(time_point);
    
    std::string r;
    boost::iostreams::stream<sink> os(r);
    os << time::year_month_day(date_point) << " "
       << time::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(time_point-date_point));
    os.flush();
    return r;
}

} // namespace time
} // namespace xbond
