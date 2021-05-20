#pragma once
#include "../vendor.h"
#include "../utility/data_view.hpp"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {

class crc32 {
    // 与 Go hash/crc32 对应 IEEE 编码结果一致
    boost::crc_optimal<32ul, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> hash_;
 public:
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    inline void update(DataView data) {
        detail::data_view dv = data;
        hash_.process_bytes(dv.data(), dv.size());
    }
    inline void update(const char* data, std::size_t size) {
        hash_.process_bytes(data, size);
    }
    inline std::uint32_t digest() {
        return hash_.checksum();
    }
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::uint32_t hash(DataView data) {
        detail::data_view dv = data;
        crc32 ctx;
        ctx.update(dv);
        return ctx.digest();
    }
};

} // namespace hash
} // namespace xbond
