#pragma once
#include <string_view>

namespace xbond {
namespace log {
// 
struct basic_record {
    virtual ~basic_record() {};
    virtual operator std::string_view() = 0;
};

} // namespace log
} // namespace xbond
