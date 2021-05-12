#pragma once
#include <type_traits>

namespace xbond {

template <typename T>
struct return_type;
// 普通函数
template <typename R, typename... Args>
struct return_type<R(*)(Args...)> {
    using type = R;
};
// 成员函数
template <typename R, typename C, typename... Args>
struct return_type<R(C::*)(Args...)> {
    using type = R;
};

} // namespace xbond
