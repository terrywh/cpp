#pragma once
#include "../vendor.h"
#include "../coroutine.hpp"

namespace xbond {
namespace time {

template <class Clock = std::chrono::steady_clock>
struct basic_ticker {
    boost::asio::basic_waitable_timer<Clock> timer;
    typename Clock::duration duration;
    template <class Executor>
    basic_ticker(Executor& e, typename Clock::duration d)
    : timer(e), duration(d) { }
    ~basic_ticker() {
        close();
    }
    template <class Callback>
    void start(Callback&& cb) {
        timer.expires_after(duration);
        timer.async_wait([this, cb = std::move(cb)] (const boost::system::error_code& error) {
            if (duration.count() > 0) start(std::move(cb));
            cb();
        });
    }
    void close() {
        reset();
        timer.cancel();
    }
    void reset() {
        duration = std::chrono::nanoseconds(0);
    }
    void reset(typename Clock::duration d) {
        duration = d;
        timer.expires_after(duration);
    }
};
using steady_ticker = basic_ticker<std::chrono::steady_clock>;

// 启动定时器，并执行回调（一次性）
template <class Executor, class Callback>
std::shared_ptr<steady_ticker> after(Executor& executor, std::chrono::steady_clock::duration interval, Callback&& cb) {
    auto context = std::make_shared<basic_ticker<std::chrono::steady_clock>>(executor, interval);
    context->start(std::forward<Callback>(cb));
    context->reset(); // 标记，不在重复执行
    return context;
}
// 启动定时器并执行回调
template <class Executor, class Callback>
std::shared_ptr<steady_ticker> tick(Executor& executor, std::chrono::steady_clock::duration interval, Callback&& cb) {
    auto context = std::make_shared<basic_ticker<std::chrono::steady_clock>>(executor, interval);
    context->start(std::forward<Callback>(cb));
    return context;
}
// 暂停当前协程（一段时间）
void sleep_for(std::chrono::steady_clock::duration duration, xbond::coroutine_handler& ch);
void sleep_for(std::chrono::steady_clock::duration duration);
} // namespace time
} // namespace xbond
