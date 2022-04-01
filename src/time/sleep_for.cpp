#include <xbond/time/sleep_for.hpp>

namespace xbond {
namespace time {
// 暂停当前协程（一段时间）
void sleep_for(std::chrono::steady_clock::duration duration, xbond::coroutine_handler& ch) {
    after(ch.executor(), duration, ch);
}
// 暂停当前协程（一段时间）
void sleep_for(std::chrono::steady_clock::duration duration) {
    xbond::coroutine_handler ch{coroutine::current()};
    sleep_for(duration, ch);
}
} // namespace time
} // namespace xbond
