#include <xbond/log/logger.hpp>
#include <xbond/log/detail/record.hpp>
#include <xbond/time/date.hpp>
#include <xbond/time/delta_clock.hpp>

namespace xbond {
namespace log {

static time::delta_clock record_time;
static const char*       record_name[] = {
    "DEBUG","TRACE","INFO","WARN","ERROR","FATAL"
};

logger::record::record(logger& l, int level, std::chrono::hours zone)
: l_(l)
, status_(0) { // 使用内存池构建实际 record 对象
    r_ = std::make_shared<detail::record>();

    std::chrono::system_clock::time_point tp = record_time; // 获取一个记录时间
    auto dp = time::floor<time::days>(tp);
    stream() << "[" << time::year_month_day(dp) << " "
        << time::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp+zone))
        << "] (" << record_name[level] << ") ";
}

std::ostream& logger::record::stream() {
    return r_->stream();
}

logger::record::~record() {
    r_->stream().put('\n');
    l_.send_record(r_);
    r_ = nullptr;
}

logger::logger(writer_t writer, std::chrono::hours offset)
: wguard_(ctx_.get_executor())
, worker_(logger::run, &ctx_)
, offset_(offset) {
    
}

logger::~logger() {
    wguard_.reset();
    worker_.join();
}

static cout_writer default_writer;

void logger::send_record(std::shared_ptr<basic_record> r) {
    boost::asio::post(ctx_, [this, record = r] () { // 当前对象不会提前销毁（等待 io_context 运行完毕）
        if (writer_.empty()) default_writer(record);
        else for (auto& writer : writer_) writer(record);
    });
}

} // namespace log
} // namespace xbond
