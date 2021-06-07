#pragma once
#include "../detail/data_view.hpp"
#include <string>
#include <openssl/opensslv.h>
#include <openssl/evp.h>
// extern "C" EVP_ENCODE_CTX *EVP_ENCODE_CTX_new(void);

namespace xbond {
namespace encoding {

class base64 {
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
    inline static std::size_t encode_size(std::size_t size) {
        return (size / 3 + 1) * 4;
    }
    /**
     * 编码到指定位置
     * 若用户需要 c_str 需要自行在长度末尾添加 '\0' 结束符
     * 注意：用户需要保证 out 指向的缓存拥有足够空间
     */
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::size_t encode(DataView str, char* out) {
        detail::data_view dv = str;
        int len;
        unsigned char *o, *b = o = reinterpret_cast<unsigned char*>(out);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
#else
        EVP_ENCODE_CTX  ctx_;
        EVP_ENCODE_CTX* ctx = &ctx_;
#endif
        EVP_EncodeInit(ctx);
        EVP_EncodeUpdate(ctx, o, &len, reinterpret_cast<const unsigned char*>(dv.data()), dv.size());
        o += len;
        EVP_EncodeFinal(ctx, o, &len);
        o += len;

        if (o > b && o[-1] == '\n') {
            return o - b - 1; // without additional newline
        }
        return o - b;
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
    inline static std::size_t decode_size(std::size_t size) {
        return (size + 3)/4 * 3;
    }
    // 解码指定数据，并将结果写入目标区域
    template <class DataView, typename = typename std::enable_if<std::is_convertible<DataView, detail::data_view>::value, DataView>::type>
    static std::size_t decode(const DataView& str, char* out) {
        detail::data_view dv = str;
        int len;
        unsigned char *o, *b = o = reinterpret_cast<unsigned char*>(out);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
#else
        EVP_ENCODE_CTX  ctx_;
        EVP_ENCODE_CTX* ctx = &ctx_;
#endif
        EVP_DecodeInit(ctx);
        EVP_DecodeUpdate(ctx, o, &len, reinterpret_cast<const unsigned char*>(dv.data()), dv.size());
        o += len;
        EVP_DecodeFinal(ctx, o, &len);
        o += len;
        if (o > b && o[-1] == '\n') {
            return o - b - 1;
        }
        return o - b;
    }
};

}
}