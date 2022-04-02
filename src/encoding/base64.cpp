#include <xbond/encoding/base64.hpp>
#include <libbase64.h>
#include <boost/beast/core/detail/base64.hpp>

namespace xbond {
namespace encoding {
/**
* 计算目标长度需求
*/
std::size_t base64::encode_size(std::size_t size) {
    return boost::beast::detail::base64::encoded_size(size);
}

/**
* 计算解码长度需求
*/
std::size_t base64::decode_size(std::size_t size) {
    return boost::beast::detail::base64::decoded_size(size);
}

std::size_t base64::encode_(detail::data_view dv, char* out) {
    std::size_t outl;
    base64_encode(dv.data(), dv.size(), out, &outl, 0);
    return outl;
}

std::size_t base64::decode_(detail::data_view dv, char* out) {
    std::size_t outl;
    base64_decode(dv.data(), dv.size(), out, &outl, 0);
    return outl;
}

} // namespace encoding
} // namespace xbond
