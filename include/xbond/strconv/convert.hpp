#pragma once
#include "../detail/to_string_view.hpp"
#include <boost/spirit/include/qi.hpp>

namespace xbond {
namespace strconv {
// string -> uint64_t
template <class S, typename = typename std::enable_if<detail::to_string_view_invokable<S>::value, S>::type>
std::uint64_t str2ul(S str) {
    std::string_view sv = detail::to_string_view(str);
    std::uint64_t n;
    boost::spirit::qi::parse(sv.data(), sv.data() + sv.size(), boost::spirit::qi::ulong_, n);
    return n;
}
// string -> uint32_t
template <class S, typename = typename std::enable_if<detail::to_string_view_invokable<S>::value, S>::type>
std::uint32_t str2ui(S str) {
    std::string_view sv = detail::to_string_view(str);
    std::uint64_t n;
    boost::spirit::qi::parse(sv.data(), sv.data() + sv.size(), boost::spirit::qi::uint_, n);
    return n;
}
// string -> uint16_t
template <class S, typename = typename std::enable_if<detail::to_string_view_invokable<S>::value, S>::type>
std::uint16_t str2us(S str) {
    std::string_view sv = detail::to_string_view(str);
    std::uint16_t n;
    boost::spirit::qi::parse(sv.data(), sv.data() + sv.size(), boost::spirit::qi::ushort_, n);
    return n;
}
// string -> uint8_t
template <class S, typename = typename std::enable_if<detail::to_string_view_invokable<S>::value, S>::type>
std::uint8_t str2uc(S str) {
    std::string_view sv = detail::to_string_view(str);
    std::uint16_t n;
    boost::spirit::qi::parse(sv.data(), sv.data() + sv.size(), boost::spirit::qi::ushort_, n);
    return n; // 对应 qi 未提供单字节数据对应类型
}

} // namespace strconv
} // namespace xbond
