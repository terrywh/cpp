#include <xbond/time/timer.hpp>

namespace xbond {
namespace time {

void sleep_for(std::chrono::steady_clock::duration duration, coroutine_handler& ch) {
    boost::asio::steady_timer timer(ch.executor());
    timer.expires_after(duration);
    timer.async_wait(ch);
}
// 使“当前”协程休眠一段时间
void sleep_for(std::chrono::steady_clock::duration duration) {
    BOOST_ASSERT_MSG(coroutine::current(), "current coroutine not exist");
    xbond::coroutine_handler ch{coroutine::current()};
    sleep_for(duration, ch);
}

} // namespace time
} // namespace xbond
