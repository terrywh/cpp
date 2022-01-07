#pragma once
#include "../../../vendor.h"

namespace xbond {
namespace net {
namespace http {

using next_handler_t = std::function<void ()>;

struct server_context {
    boost::beast::tcp_stream stream;
    boost::beast::http::request<boost::beast::http::empty_body> header;
};

template <class T>
struct is_server_handler {
    template <typename U> static auto test(int) -> typename std::enable_if<
        std::is_constructible<U, boost::beast::tcp_stream&>::value && 
        std::is_invocable<U, next_handler_t>::value,
        std::true_type>::type;
    template <typename> static std::false_type test(...);
    constexpr static bool value = !std::is_same<decltype(test<T>(0)), std::false_type>::value;
};

class server_handler {
    
};

} // namespace http
} // namespace net
} // namespace xbond
