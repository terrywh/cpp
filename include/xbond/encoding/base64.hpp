#pragma once
#include "../detail/data_view.hpp"
#include <string>

namespace xbond {
namespace encoding {

class base64 {
    static std::size_t encode_(detail::data_view dv, char* out);
    static std::size_t decode_(detail::data_view dv, char* out);

public:
    // 编码并返回变后的字符串
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::string encode(DataView str) {
        detail::data_view dv = str;
        std::size_t len = encode_size(dv.size());
        std::string enc(len, '\0');
        
        len = encode(dv, const_cast<char*>(enc.data()));

        enc.resize(len); // additional \0 is written
        return enc;
    }
    /**
     * 计算目标长度需求
     */
    static std::size_t encode_size(std::size_t size);
    /**
     * 编码到指定位置
     * 若用户需要 c_str 需要自行在长度末尾添加 '\0' 结束符
     * 注意：用户需要保证 out 指向的缓存拥有足够空间
     */
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::size_t encode(DataView str, char* out) {
        return encode_(str, out);
    }
    /**
     * 编码指定数据
     */
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::string decode(DataView str) {
        detail::data_view dv = str;
        int len = decode_size(dv.size());
        std::string dec(len, '\0');

        len = decode(dv, const_cast<char*>(dec.data()));

        dec.resize(len);
        return dec;
    }
    /**
     * 计算解码长度需求
     */
    static std::size_t decode_size(std::size_t size);
    // 解码指定数据，并将结果写入目标区域
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::size_t decode(const DataView& str, char* out) {
        return decode_(str, out);
    }
};

} // namespace encoding
} // namespace xbond
