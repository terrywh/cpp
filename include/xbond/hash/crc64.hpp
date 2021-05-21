#pragma once
#include "../vendor.h"
#include "../detail/data_view.hpp"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {

class crc64 {
    // 与 Golang hash/crc64 定义 ECMA 编码结果一致
    boost::crc_optimal<64ul, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, true, true>
        hash_;
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
    inline std::uint64_t digest() {
        return hash_.checksum();
    }
    // 一次性对指定数据区块进行哈希计算并返回计算结果
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::uint64_t hash(DataView data) {
        detail::data_view dv = data;
        crc64 ctx;
        ctx.update(dv);
        return ctx.digest();
    }
};

} // namespace hash
} // namespace xbond
