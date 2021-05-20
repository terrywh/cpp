#pragma once
#include "../vendor.h"
#include "../utility/data_view.hpp"
#include <openssl/sha.h>

namespace xbond {
namespace hash {

class sha256 {
    SHA256_CTX hash_;
 public:
    sha256() {
        SHA256_Init(&hash_);
    }
    template <class StringView, typename = typename std::enable_if<std::is_convertible<StringView, std::string_view>::value, StringView>::type>
    void update(StringView s) {
        std::string_view sv = s;
        SHA256_Update(&hash_, sv.data(), sv.size());
    }
    void update(const char* data, std::size_t size) {
        SHA256_Update(&hash_, data, size);
    }
    std::array<std::uint8_t, 32> digest() {
        std::array<std::uint8_t, 32> hash;
        SHA256_Final(hash.data(), &hash_);
        return hash;
    }
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::array<std::uint8_t, 32> hash(DataView data) {
        detail::data_view dv = data;
        std::array<std::uint8_t, 32> hash;
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, dv.data(), dv.size());
        SHA256_Final(hash.data(), &ctx);
        return hash;
    }
};

} // namespace hash
} // namespace xbond
