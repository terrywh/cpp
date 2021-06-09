#include <xbond/log/logger.hpp>
#include <xbond/time/date.hpp>
#include <xbond/time/delta_clock.hpp>

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

file_writer::file_writer(const char* file, bool create_directory) {
    if (create_directory) {
        boost::filesystem::path path(file);
        boost::filesystem::create_directories(path.parent_path());
    }
    // 打开文件
    file_ = std::make_shared<std::ofstream>(file, std::ios_base::ate | std::ios_base::out | std::ios_base::app);
    if (!file_->is_open()) 
        throw std::runtime_error("cannot create/open file for append");
}

void file_writer::operator()(const char* data, std::size_t size) {
    file_->write(data, size);
    file_->flush(); // 按写入条目输出
}

} // namespace log
} // namespace xbond
