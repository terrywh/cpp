#pragma once
#include <type_traits>

namespace xbond {
namespace encoding {
/**
 * 判定指定类型是否为哈希计算封装，大致类型约定如下形式：
 * class T {
 * public:
 *      void update(const char* data, std::size_t size);
 *      R digest();
 * };
 * class R { ... };
 */
template <class T>
struct is_hasher {
    template <typename U> static auto test(int) -> decltype(
        std::declval<U>().update((const char*)nullptr, (std::size_t)0ul), // 提交数据
        !std::is_same<decltype(std::declval<U>().digest()), void>::value, // 编解码并返回数据
        std::true_type());
    template <typename> static std::false_type test(...);
    constexpr static bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

} // namespace encoding
} // namespace xbond
