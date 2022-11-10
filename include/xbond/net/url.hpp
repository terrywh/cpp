#pragma once
#include "address.hpp"
#include "../detail/to_string_view.hpp"

namespace xbond {
namespace net {

/// 封装对标准形式的 URL 对象，提供基本的解析功能，并提供以下数据属性访问：
class url {
    std::string raw;
 public:
    std::string_view scheme;
    std::string_view username;
    std::string_view password;
    std::string_view domain;
    std::string_view port;
    std::string_view path;  // 含起始 "/" 符号
    std::string_view query; // 含起始 "?" 符号
    std::string_view hash;  // 含起始 "#" 符号

    operator xbond::net::address() const;

    friend url parse_url(std::string&& url);
    friend url parse_url(std::string_view url, bool copy);
    // 
    friend std::ostream& operator<<(std::ostream& os, const url& u);
};

const url& empty_url();

url parse_url(std::string_view str, bool copy = false);
inline url parse_url(std::string&& str) {
    url u = parse_url(std::string_view{str}, false);
    u.raw = std::move(str);
    return u;
}
inline url parse_url(const std::string& str, bool copy = false) {
    return parse_url(std::string_view{str}, copy);
}
inline url parse_url(const char* str, bool copy = false) {
    return parse_url(std::string_view{str}, copy);
}
template <class S, typename = typename std::enable_if<xbond::detail::convertible_to_string_view<S>::value, S>::type>
url parse_url(const S& str, bool copy = false) {
    std::string_view sv = xbond::detail::to_string_view(str);
    return parse_url(sv, copy);
}

} // namespace net
} // namespace xbond
