#include <xbond/time/timer.hpp>

namespace xbond {
namespace time {

void sleep(std::chrono::steady_clock::duration duration) {
    coroutine_handler ch {xbond::coroutine::current()};
    boost::asio::steady_timer tm {ch.executor(), duration};
    tm.async_wait(ch);
}

} // namespace time
} // namespace xbond
