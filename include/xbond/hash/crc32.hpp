#pragma once
#include "../vendor.h"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {

class crc32 {
    // 与 Go hash/crc32 对应 IEEE 编码结果一致
    boost::crc_optimal<32ul, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> hash_;
 public:
    template <class Span>
    void update(Span sv) {
        hash_.process_bytes(sv.data(), sv.size());
    }
    void update(const char* data, std::size_t size) {
        hash_.process_bytes(data, size);
    }
    std::uint32_t digest() {
        return hash_.checksum();
    }
    template <class Span>
    static std::uint32_t hash(Span sv) {
        crc32 ctx;
        ctx.update(sv);
        return ctx.digest();
    }
};

} // namespace hash
} // namespace xbond
