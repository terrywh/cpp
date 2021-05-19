#pragma once
#include "../vendor.h"

namespace xbond {
namespace log {

#define XBOND_LOGGER_RECORD_STREAM(_logger_, _level_) for (auto record = \
    (_logger_).open_record(xbond::log::logger::_level_); !!record;) record.stream()

/**
 * 日志对象
 * @remark 同一对象支持多线程调用；
 */
class logger {
    std::function<void (const char* data, std::size_t size)> writer_;
    std::chrono::hours offset_;
 public:
    enum level_type {
        DEBUG   = 0x00,
        TRACE   = 0x01,
        INFO    = 0x02,
        // INFORMATION = 0x02,
        WARN    = 0x03,
        WARNING = 0x03,
        ERROR   = 0x04,
        FATAL   = 0x05,
    };
    // 日志记录
    class record {
    public:
        enum {
            STATUS_ONCE = 0x01,
        };
        record(const record& r) = delete;

        ~record() {
            stream_.put('\n');
            logger_.send_record(*this);
        }

        std::ostream& stream() {
            return stream_;
        }

        operator bool() {
            bool s = !(status_ & STATUS_ONCE);
            status_ |= STATUS_ONCE;
            return s;
        }

    private:
        explicit record(logger& logger, level_type level);
        logger&           logger_;
        int               status_;
        std::stringstream stream_;

        friend class logger;
    };
    // 指定输出器
    template <class Writer>
    logger(Writer&& writer, std::chrono::hours zone_offset = std::chrono::hours(8))
    : writer_(writer)
    , offset_(zone_offset) {}
    // 默认日志器（输出到标准输出）
    logger(std::chrono::hours zone_offset = std::chrono::hours(8))
    : logger([] (const char* data, std::size_t size) {
        std::cout.write(data, size);
        std::cout.flush();
    }, zone_offset) {}
    //
    record open_record(level_type lvl);
    void   send_record(record& record);
    std::chrono::system_clock::time_point time_record() const;
    template <class Writer>
    void writer(Writer&& w) { writer_ = w; }
};

class file_writer {
    std::shared_ptr<std::ofstream> file_;

 public:
    file_writer(const char* file, bool create_directory = false);
    void operator()(const char* data, std::size_t size);
};

} // namespace log
} // namespace xbond
