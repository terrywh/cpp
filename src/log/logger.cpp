#include "../../include/xbond/log/logger.hpp"
#include "../../include/xbond/time/date.hpp"
#include "../../include/xbond/time/delta_clock.hpp"

namespace xbond {
namespace log {

static const char* logger_level_strs[] = {
    "DEBUG","TRACE","INFO","WARN","ERROR","FATAL"
};

logger::record::record(logger& logger, level_type level)
: logger_(logger), status_(0) {
    auto tp = logger.time_record();
    auto dp = time::floor<time::days>(tp);
    
    stream_ << "[" << time::year_month_day(dp) << " "
        << time::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp+logger.offset_))
        << "] (" << logger_level_strs[static_cast<int>(level)] << ") ";
}

logger::record logger::open_record(level_type lvl) {
    return logger::record(*this, lvl);
}
//
void logger::send_record(record& record) {
    auto str = record.stream_.str();
    writer_(str.data(), str.size());
}

std::chrono::system_clock::time_point logger::time_record() const {
    static time::delta_clock clock;
    return clock;
}

} // namespace log
} // namespace xbond
