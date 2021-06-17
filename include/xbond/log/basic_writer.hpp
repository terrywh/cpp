#pragma once
#include "../vendor.h"
#include "basic_record.hpp"

namespace xbond {
namespace log {
    
/**
 * 判定类型是否可被用于日志输出
 * 大致约定如下：
 * T writer;
 * basic_record* r;
 * writer(r);
 */
template <class T>
struct is_writer {
    template <typename U> static auto test(int) -> typename std::invoke_result<U, std::shared_ptr<basic_record>>::type;
    template <typename> static std::false_type test(...);
    constexpr static bool value = !std::is_same<decltype(test<T>(0)), std::false_type>::value;
};

// 通用 writer 定义
typedef std::function<void (std::shared_ptr<basic_record> r)> writer_t;

} // namespace log
} // namespace xbond
