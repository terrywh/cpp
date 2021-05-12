#pragma once
#include <boost/utility/string_view.hpp>
#include <boost/spirit/include/qi.hpp>

namespace xbond {
namespace strconv {

template <class StringView>
std::uint64_t str2ul(StringView str) {
    std::uint64_t n;
    boost::spirit::qi::parse(str.data(), str.data() + str.size(), boost::spirit::qi::ulong_, n);
    return n;
}

template <class StringView>
std::uint32_t str2ui(StringView str) {
    std::uint64_t n;
    boost::spirit::qi::parse(str.data(), str.data() + str.size(), boost::spirit::qi::uint_, n);
    return n;
}

template <class StringView>
std::uint16_t str2us(StringView str) {
    std::uint16_t n;
    boost::spirit::qi::parse(str.data(), str.data() + str.size(), boost::spirit::qi::ushort_, n);
    return n;
}

template <class StringView>
std::uint8_t str2uc(StringView str) {
    std::uint16_t n;
    boost::spirit::qi::parse(str.data(), str.data() + str.size(), boost::spirit::qi::ushort_, n);
    return n; // 对应 qi 未提供单字节数据对应类型
}

} // namespace strconv
} // namespace xbond
