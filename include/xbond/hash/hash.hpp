#pragma once
#include "../detail/data_view.hpp"
#include "algorithm/algorithm.hpp"
#include "algorithm/openssl.hpp"
#include "algorithm/boost.hpp"

namespace xbond {
namespace hash {

template <typename Hash, typename R = typename std::enable_if<algorithm::is_algorithm<Hash>::value>::type>
class hash {
    using hash_type = typename std::decay<Hash>::type;
    hash_type hash_;
    using value_type = typename hash_type::value_type;
 public:
    // （继续）计算数据区块
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    inline void update(DataView data) {
        detail::data_view dv = data;
        hash_.update(dv.data(), dv.size());
    }
    // （继续）计算数据区块
    inline void update(const char* data, std::size_t size) {
        hash_.update(data, size);
    }
    // （完毕）返回哈希结果
    inline value_type digest() {
        return hash_.digest();
    }
    // 一次性对指定数据区块进行哈希计算并返回计算结果
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static value_type digest(DataView data) {
        detail::data_view dv = data;
        hash_type h;
        h.update(dv.data(), dv.size());
        return h.digest();
    }
};

using crc16  = hash<algorithm::crc16>;
using crc32  = hash<algorithm::crc32>;
using crc64  = hash<algorithm::crc64>;
using md5    = hash<algorithm::md5>;
using sha1   = hash<algorithm::sha1>;
using sha256 = hash<algorithm::sha256>;
using sha512 = hash<algorithm::sha512>;

} // namespace hash
} // namespace xbond
