#pragma once
#include "../../vendor.h"
#include "../writer.hpp"

namespace xbond {
namespace log {
namespace detail {

// 日志记录
class record : public basic_record {
    std::stringstream stream_;
    std::string output_;
public:
    void* operator new(std::size_t size); // 实现 fast pimpl 语义
    void operator delete(void* p);
    record() = default;
    // 禁止复制
    record(const record& r) = delete;
    // 输出流
    std::ostream& stream() {
        return stream_;
    }
    // 输出
    operator std::string_view() override {
        if (output_.empty()) output_ = stream_.str();
        return output_;
    }
};

} // namespace detail
} // namespace log
} // namespace xbond
