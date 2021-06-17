#pragma once
#include "../vendor.h"
#include "boost/asio/executor_work_guard.hpp"
#include "boost/asio/io_context.hpp"
#include "writer.hpp"

#define XBOND_LOGGER_RECORD_STREAM(_logger_, _level_) for (auto record = \
    (_logger_).open_record(xbond::log::logger::_level_); !!record;) record.stream()

namespace xbond {
namespace log {
namespace detail {
class record;
};
/**
 * 日志对象
 * @remark 同一对象支持多线程调用；
 */
class logger {
    boost::asio::io_context ctx_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> wguard_;
    std::thread worker_;
    std::vector<writer_t> writer_;
    std::chrono::hours    offset_;

    static void run(boost::asio::io_context* io) {
        io->run();
    }

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
        logger& l_;
        std::shared_ptr<detail::record> r_;

    public:
        enum {
            STATUS_ONCE = 0x01,
        };
        // 禁止复制
        record(const record& r) = default;
        record(record&& r) = default;
        // 销毁时发送给 logger 存储记录
        ~record();
        // 输出流
        std::ostream& stream();
        // 用于禁止同一 Record 对象被多次使用
        operator bool() {
            bool s = !(status_ & STATUS_ONCE);
            status_ |= STATUS_ONCE;
            return s;
        }

    private:
        explicit record(logger& l, int level, std::chrono::hours zone);
        int status_;

        friend class logger;
    };
    logger(writer_t w = cout_writer{}, std::chrono::hours offset = std::chrono::hours(8));
    ~logger();
    // 打开一个记录
    record open_record(int level) {
        return logger::record (*this, level, offset_);
    }
    void send_record(std::shared_ptr<basic_record> r);

    // 输出方法
    void append(writer_t writer) { writer_.emplace_back(std::move(writer)); }
    void clear() { writer_.clear(); }
    // 时区偏移
    void offset(std::chrono::hours offset) { offset_ = offset; }
    std::chrono::hours offset() const { return offset_; }
};


} // namespace log
} // namespace xbond
