#pragma once
#include "../vendor.h"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {

class crc64 {
    // 与 Golang hash/crc64 定义 ECMA 编码结果一致
    boost::crc_optimal<64ul, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, true, true>
        hash_;
 public:
    template <class Span>
    void update(Span sv) {
        hash_.process_bytes(sv.data(), sv.size());
    }
    std::uint64_t digest() {
        return hash_.checksum();
    }
    template <class Span>
    static std::uint64_t hash(Span sv) {
        crc64 ctx;
        ctx.update(sv);
        return ctx.digest();
    }
};

} // namespace hash
} // namespace xbond
