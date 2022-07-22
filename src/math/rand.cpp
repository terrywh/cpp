#include <xbond/math/rand.hpp>

namespace xbond {
namespace math {
namespace rand {
namespace detail {
std::random_device& device() {
    static std::random_device device;
    return device;
}
} // namespace detail


} // namespace rand
} // namespace math
} // namespace xbond