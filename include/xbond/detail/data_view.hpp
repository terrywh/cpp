#pragma once
#include <type_traits>
#include <cstring>

namespace xbond {
namespace detail {

/**
 * 判定一个容器对象是否可以被用于查看对应的二进制数据
 * 大致约定如下：
 * class T {
 *  public:
 *     typedef <CompleteType> value_type;
 *     const void* data() const;
 *     std::size_t size() const;
 * }
 */
template <class T>
struct is_data_container {
    template <typename U> static auto test(int) -> typename std::enable_if<
            std::is_convertible<decltype(std::declval<U>().data()), const void*>::value && // 数据指针
            std::is_convertible<decltype(std::declval<U>().size()), std::size_t>::value,   // 数据长度
        typename U::value_type>::type;
    template <typename> static std::false_type test(...);
    constexpr static bool value = !std::is_same<decltype(test<T>(0)), std::false_type>::value;
};

/**
 * 与 string_view 类似，但用于二进制数据
 */
class data_view {
    const char* data_;
    std::size_t size_;

 public:
    data_view(const char* data)
    : data_(data), size_(std::strlen(data)) {}

    data_view(const void* data, std::size_t size)
    : data_(static_cast<const char*>(data)), size_(size) {}

    data_view(const void* begin, const void* end)
    : data_(static_cast<const char*>(begin)), size_(static_cast<const char*>(end) - static_cast<const char*>(begin)) {}

    template <class Container, typename = typename std::enable_if<is_data_container<Container>::value, Container>::type>
    data_view(const Container& c)
    : data_(static_cast<const char*>(static_cast<const void*>(c.data()))), size_(c.size() * sizeof(typename Container::value_type)) {}

    template <class Element, std::size_t N>
    data_view(const Element (&array)[N])
    : data_(array), size_(N * sizeof(Element)) {}
    // TODO: 支持更多形态的构建（转换机制）
    // 二进制数据指针
    const char* data() const { return data_; }
    // 二进制数据大小（字节数）
    std::size_t size() const { return size_; }

    const char* begin() const { return static_cast<const char*>(data_); }
    const char* end() const { return static_cast<const char*>(data_) + size_; }

};

} // namespace detail
} // namespace xbond
