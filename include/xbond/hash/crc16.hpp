#pragma once
#include "../vendor.h"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {

class crc16 {
    boost::crc_optimal<16ul, 0x8005, 0, 0, true, true>
        hash_;
 public:
    template <class Span>
    void update(Span sv) {
        hash_.process_bytes(sv.data(), sv.size());
    }
    void update(const char* data, std::size_t size) {
        hash_.process_bytes(data, size);
    }
    std::uint16_t digest() {
        return hash_.checksum();
    }
    template <class Span>
    static std::uint16_t hash(Span sv) {
        crc16 ctx;
        ctx.update(sv);
        return ctx.digest();
    }
};

} // namespace hash
} // namespace xbond
