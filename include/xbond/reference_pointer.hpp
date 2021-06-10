#pragma once
#include <boost/system/error_code.hpp>
#include <memory>

namespace xbond {
// 用于包裹一个含有IO对象的业务实例 带有 Onwer 的指针，并代理对其对应 operator() 的调用
template <class Ptr>
class reference_pointer {
    Ptr p_;

 public:
    reference_pointer(Ptr&& p)
    : p_(std::move(p)) {}

    template <class T = Ptr>
    auto operator ()(const boost::system::error_code& error)
            -> decltype(std::declval<T>()->operator()(std::declval<boost::system::error_code>())) {
        p_->operator()(error);
    }
    template <class T = Ptr>
    auto operator ()(const boost::system::error_code& error, std::size_t size) 
            -> decltype(std::declval<T>()->operator()(std::declval<boost::system::error_code>(), 0ul)) {
        p_->operator()(error, size);
    }
};

} // namespace xbond
