#pragma once
#include "../coroutine.hpp"
#include <boost/asio/steady_timer.hpp>
#include <chrono>

namespace xbond {
namespace time {
// 启动定时器，并执行回调（一次性）
template <class Executor, class Callback>
void after(Executor&& executor, std::chrono::steady_clock::duration interval, Callback&& cb) {
    auto timer = std::make_shared<boost::asio::steady_timer>(std::forward<Executor>(executor), interval);
    timer->async_wait(std::forward<Callback>(cb));
}
// 启动定时器并执行回调（一直重复，除非 io 停止）
template <class Executor, class Callback>
void tick(Executor&& executor, std::chrono::steady_clock::duration interval, Callback&& cb) {
    auto timer = std::make_shared<boost::asio::steady_timer>(std::forward<Executor>(executor), interval);
    timer->async_wait(std::forward<Callback>(cb));
}
} // namespace time
} // namespace xbond
