#pragma once
#include "../detail/to_string_view.hpp"
#include "detail/parse_query_handler.hpp"
#include <vspp/keyvalue/basic_parser.hpp>

namespace xbond {
namespace net {

template <class S, class Handler, typename = typename std::enable_if<xbond::detail::convertible_to_string_view<S>::value, S>::type>
void parse_query(S query, Handler&& handler) {
    std::string_view sv = xbond::detail::to_string_view(query);
    if (sv[0] == '?') sv = sv.substr(1);
    if constexpr (std::is_invocable<Handler, std::pair<std::string, std::string>&&>::value) {
        vspp::keyvalue::basic_parser<detail::parse_query_handler_ex<Handler>> p1 { detail::parse_query_handler_ex<Handler>(handler), "\0\0=\0\0&"};
        p1.parse(sv);
        p1.end();
    }
    else if constexpr (vspp::is_handler<Handler>::value) {
        vspp::keyvalue::basic_parser<detail::parse_query_handler<Handler>> p1 { detail::parse_query_handler<Handler>(handler), "\0\0=\0\0&"};
        p1.parse(sv);
        p1.end();
    }
}

} // namespace net
} // namespace xbond
