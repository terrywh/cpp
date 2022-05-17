#pragma once
#include "../../encoding/percent.hpp"

namespace xbond {
namespace net {
namespace detail {

template <class Handler>
struct parse_query_handler {
    Handler& handler_;

    parse_query_handler(Handler& handler): handler_(handler) {}
    using field_type = typename Handler::field_type;
    using value_type = typename Handler::value_type;
    void on_entry(std::pair<field_type, value_type>&& entry) {
        std::pair<field_type, value_type> e (std::move(entry));
        e.second = xbond::encoding::percent::decode(e.second);
        handler_.on_entry(std::move(e));
    }
};

template <class Handler>
struct parse_query_handler_ex {
    Handler handler_;

    parse_query_handler_ex(const Handler& handler): handler_(handler) {}
    using field_type = std::string;
    using value_type = std::string;
    void on_entry(std::pair<field_type, value_type>&& entry) {
        std::pair<field_type, value_type> e (std::move(entry));
        e.second = xbond::encoding::percent::decode(e.second);
        handler_(std::move(e));
    }
};

}
} // namespace net 
} // namespace xbond
