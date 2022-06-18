#pragma once
#include "../coroutine.hpp"
#include "detail/timer_context.hpp"
#include <chrono>

namespace xbond {
namespace time {
// 启动定时器，并执行回调（一次性）
template <class Executor, class CompletionToken>
void after(Executor&& executor, std::chrono::steady_clock::duration interval, CompletionToken&& cb) {
    auto w = std::make_shared<timer_context<CompletionToken>>(std::forward<Executor>(executor), std::forward<CompletionToken>(cb));
    w->timer.expires_after(interval);
    w->timer.async_wait([w] (const boost::system::error_code& error) {
        w->completion.completion_handler(error);
    });
    w->completion.result.get();
}
// 启动定时器并执行回调（一直重复，除非 io 停止）
template <class Executor, class CompletionToken>
void tick(Executor&& executor, std::chrono::steady_clock::duration interval, CompletionToken&& cb) {
    auto w = std::make_shared<timer_context<CompletionToken>>(std::forward<Executor>(executor), std::forward<CompletionToken>(cb));
    w->timer.expires_after(interval);
    w->timer.async_wait([w] (const boost::system::error_code& error) {
        w->completion.completion_handler(error);
    });
    w->completion.result.get();
}
} // namespace time
} // namespace xbond
