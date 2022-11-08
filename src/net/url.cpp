#include <xbond/net/url.hpp>

namespace xbond {
namespace net {

const net::url& empty_url() {
    static net::url empty;
    return empty;
}

} // namespace net
} // namespace xbond
