#include <xbond/os/env.hpp>

namespace xbond {
namespace os {

static boost::filesystem::path executable_path = [] () {
    char buffer[1024];
    ssize_t length;
#ifdef __linux__
    // std::memset(buffer, 0, sizeof(buffer));
    length = ::readlink("/proc/self/exe", buffer, sizeof(buffer));
#else
    buffer[0] = '.';
    length = 1;
#endif
    buffer[length] = '\0';
    return boost::filesystem::path {buffer};
} ();

const boost::filesystem::path& executable() {
    return executable_path;
}

} // namespace os
} // namespace xbond
