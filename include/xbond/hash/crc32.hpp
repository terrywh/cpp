#pragma once
#include "../vendor.h"
#include "../detail/data_view.hpp"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {

class crc32 {
    // 与 Go hash/crc32 对应 IEEE 编码结果一致
    boost::crc_optimal<32ul, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> hash_;
 public:
     // （继续）计算数据区块
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    inline void update(DataView data) {
        detail::data_view dv = data;
        hash_.process_bytes(dv.data(), dv.size());
    }
     // （继续）计算数据区块
    inline void update(const char* data, std::size_t size) {
        hash_.process_bytes(data, size);
    }
    // （完毕）返回哈希结果
    inline std::uint32_t digest() {
        return hash_.checksum();
    }
    // 一次性对指定数据区块进行哈希计算并返回计算结果
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
