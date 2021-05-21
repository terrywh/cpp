#pragma once
#include "../vendor.h"
#include "../detail/data_view.hpp"
#include <openssl/sha.h>

namespace xbond {
namespace hash {

class sha1 {
    SHA_CTX hash_;
 public:
    sha1() {
        SHA1_Init(&hash_);
    }
    // （继续）计算数据区块
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    inline void update(DataView data) {
        detail::data_view dv = data;
        SHA1_Update(&hash_, dv.data(), dv.size());
    }
    // （继续）计算数据区块
    inline void update(const char* data, std::size_t size) {
        SHA1_Update(&hash_, data, size);
    }
    // （完毕）返回哈希结果
    inline std::array<std::uint8_t, 20> digest() {
        std::array<std::uint8_t, 20> hash;
        SHA1_Final(hash.data(), &hash_);
        return hash;
    }
    // 一次性对指定数据区块进行哈希计算并返回计算结果
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::array<std::uint8_t, 20> hash(DataView data) {
        detail::data_view dv = data;
        std::array<std::uint8_t, 20> hash;
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, dv.data(), dv.size());
        SHA1_Final(hash.data(), &ctx);
        return hash;
    }
};

} // namespace hash
} // namespace xbond
