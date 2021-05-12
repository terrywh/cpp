#pragma once
#include <iterator>

namespace xbond {
namespace encoding {

template <class T>
class pointer_back_iterator {
public:
    using value_type = typename std::decay<T>::type;
    pointer_back_iterator(value_type* p): p_(p) {}
    pointer_back_iterator& operator ++() { ++p_; return *this; }
    value_type& operator*() const { return *p_; }
private:
    value_type* p_;
};
} // namespace encoding
} // namespace xbond

namespace std {
// 提供 iterator  
template <class T>
struct iterator_traits<xbond::encoding::pointer_back_iterator<T>> {
    using value_type = typename xbond::encoding::pointer_back_iterator<T>::value_type;
};
} // namespace std
