#pragma once
#include "detail/netlink_route.hpp"

namespace xbond {
namespace net {

template <class Handler>
void foreach_device(Handler&& handler) {
    detail::netlink_route nr;
    nr.foreach_device(std::forward<Handler>(handler));
}

} // namespace net
} // namespace xbond
