#pragma once
#include "../vendor.h"
#include <openssl/sha.h>

namespace xbond {
namespace hash {

class sha1 {
    SHA_CTX hash_;
 public:
    sha1() {
        SHA1_Init(&hash_);
    }
    template <class Span>
    void update(Span sv) {
        SHA1_Update(&hash_, sv.data(), sv.size());
    }
    std::array<std::uint8_t, 20> digest() {
        std::array<std::uint8_t, 20> hash;
        SHA1_Final(hash.data(), &hash_);
        return hash;
    }
    template <class Span>
    static std::array<std::uint8_t, 20> hash(Span sv) {
        std::array<std::uint8_t, 20> hash;
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        SHA1_Update(&ctx, sv.data(), sv.size());
        SHA1_Final(hash.data(), &ctx);
        return hash;
    }
};

} // namespace hash
} // namespace xbond
