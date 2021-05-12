#pragma once
#include "../vendor.h"
#include <openssl/sha.h>

namespace xbond {
namespace hash {

class sha256 {
    SHA256_CTX hash_;
 public:
    sha256() {
        SHA256_Init(&hash_);
    }
    template <class StringView>
    void update(StringView sv) {
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
    template <class Span>
    static std::array<std::uint8_t, 32> hash(Span sv) {
        std::array<std::uint8_t, 32> hash;
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, sv.data(), sv.size());
        SHA256_Final(hash.data(), &ctx);
        return hash;
    }
};

} // namespace hash
} // namespace xbond
