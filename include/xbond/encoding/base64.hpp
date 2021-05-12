#pragma once
#include <string>
#include <openssl/evp.h>
// extern "C" EVP_ENCODE_CTX *EVP_ENCODE_CTX_new(void);

namespace xbond {
namespace encoding {

class base64 {
public:
    // 编码并返回变后的字符串
    template <class Span>
    static std::string encode(Span str) {
        std::size_t len = encode_size(str.size());
        std::string enc(len, '\0');
        
        len = encode(str, const_cast<char*>(enc.data()));

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
    template <class Span>
    static std::size_t encode(Span str, char* out) {
        int len;
        unsigned char *o, *b = o = reinterpret_cast<unsigned char*>(out);

        EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
        EVP_EncodeInit(ctx);
        EVP_EncodeUpdate(ctx, o, &len, reinterpret_cast<const unsigned char*>(str.data()), str.size());
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
    template <class Span>
    static std::string decode(Span str) {
        int len = decode_size(str.size());
        std::string dec(len, '\0');

        len = decode(str, const_cast<char*>(dec.data()));

        dec.resize(len);
        return dec;
    }
    /**
     * 计算解码长度需求
     */
    inline static std::size_t decode_size(std::size_t size) {
        return (size + 3)/4 * 3;
    }

    template <class Span>
    static std::size_t decode(const Span& str, char* out) {
        int len;
        unsigned char *o, *b = o = reinterpret_cast<unsigned char*>(out);

        EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
        EVP_DecodeInit(ctx);
        EVP_DecodeUpdate(ctx, o, &len, reinterpret_cast<const unsigned char*>(str.data()), str.size());
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