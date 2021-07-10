#pragma once
#include "../detail/to_string_view.hpp"
#include <boost/spirit/include/qi.hpp>

namespace xbond {
namespace strconv {
// string -> uint64_t
template <class S, typename = typename std::enable_if<detail::convertible_to_string_view<S>::value, S>::type>
class parse_string {
    union {
        std::uint64_t u_;
        std::int64_t  s_;
    };

public:
    parse_string(const S& str) {
        std::string_view sv = detail::to_string_view(str);
        if (sv.empty()) s_ = 0l;
        else if (sv[0] == '-') 
            boost::spirit::qi::parse(sv.data(), sv.data() + sv.size(), boost::spirit::qi::long_,  s_);
        else 
            boost::spirit::qi::parse(sv.data(), sv.data() + sv.size(), boost::spirit::qi::ulong_, u_);
    }
    operator std::uint64_t() const { return u_; }
    operator std::int64_t() const { return s_; }
    operator unsigned int() const { return u_; }
    operator int() const { return s_; }
    operator unsigned short() { return u_; }
    operator short() const { return s_; }
};

} // namespace strconv
} // namespace xbond
