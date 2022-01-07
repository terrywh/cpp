#pragma once
#include "../../detail/data_view.hpp"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <array>
#include <cstdint>

namespace xbond {
namespace hash {
namespace algorithm {

#define CLASS_OPENSSL_HASH_ALGORITHM(NAME, CONTEXT_PREFIX, FUNCTION_PREFIX) class NAME { \
    CONTEXT_PREFIX ## _CTX ctx_;                                        \
 public:                                                                \
    using value_type = std::array<std::uint8_t, CONTEXT_PREFIX ## _DIGEST_LENGTH>; \
    NAME () { FUNCTION_PREFIX ## _Init(&ctx_); }                        \
    inline void update(const char* data, std::size_t size) { FUNCTION_PREFIX ## _Update(&ctx_, data, size); } \
    inline value_type digest() {                                       \
        value_type hash;                                               \
        FUNCTION_PREFIX ## _Final(hash.data(), &ctx_);                  \
        return hash;                                                    \
    }                                                                   \
}

CLASS_OPENSSL_HASH_ALGORITHM(md5, MD5, MD5);
CLASS_OPENSSL_HASH_ALGORITHM(sha1, SHA, SHA1);
CLASS_OPENSSL_HASH_ALGORITHM(sha256, SHA256, SHA256);
CLASS_OPENSSL_HASH_ALGORITHM(sha512, SHA512, SHA512);

} // namespace algorithm
} // namespace hash
} // namespace xbond
