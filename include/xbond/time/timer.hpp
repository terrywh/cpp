#pragma once
#include "../vendor.h"
#include "../coroutine.hpp"

namespace xbond {
namespace time {

template <class Clock = std::chrono::steady_clock>
struct basic_ticker: public std::enable_shared_from_this<basic_ticker<Clock>> {
    boost::asio::basic_waitable_timer<Clock> timer;
    typename Clock::duration duration;
    bool repeat;
    template <class Executor>
    basic_ticker(Executor& e, typename Clock::duration d, bool r = true)
    : timer(e), duration(d), repeat(r) { }
    ~basic_ticker() {
        close();
    }
    template <class Callback>
    void start(Callback&& cb) {
        timer.expires_after(duration);
        timer.async_wait([this, self = this->shared_from_this(), cb = std::move(cb)] (const boost::system::error_code& error) {
            if (error) return;
            cb();
            if (repeat) start(std::move(cb));
        });
    }
    template <class Callback>
    void start(Callback&& cb, typename Clock::duration d) {
        duration = d;
        timer.cancel();
        start(std::move(cb));
    }
    void close() {
        repeat = false;
        // duration = std::chrono::nanoseconds(0);
        timer.cancel();
    }
};
using steady_ticker = basic_ticker<std::chrono::steady_clock>;

// 启动定时器，并执行回调（一次性）
template <class Executor, class Callback>
std::shared_ptr<basic_ticker<std::chrono::steady_clock>> after(Executor& executor, std::chrono::steady_clock::duration interval, Callback&& cb) {
    auto context = std::make_shared<basic_ticker<std::chrono::steady_clock>>(executor, interval, false);
    context->start(std::forward<Callback>(cb));
    return context;
}
// 启动定时器并执行回调
template <class Executor, class Callback>
std::shared_ptr<basic_ticker<std::chrono::steady_clock>> tick(Executor& executor, std::chrono::steady_clock::duration interval, Callback&& cb) {
    auto context = std::make_shared<basic_ticker<std::chrono::steady_clock>>(executor, interval);
    context->start(std::forward<Callback>(cb));
    return context;
}
// 暂停当前协程（一段时间）
void sleep(std::chrono::steady_clock::duration duration);
} // namespace time
} // namespace xbond
