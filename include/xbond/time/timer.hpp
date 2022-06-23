#pragma once
#include "../coroutine.hpp"
#include "detail/do_tick.hpp"
#include <chrono>

namespace xbond {
namespace time {


// 启动定时器，并执行回调（一次性）
template <class Executor, class CompletionToken, class Signature = void (boost::system::error_code)>
void after(Executor& executor, std::chrono::steady_clock::duration interval, CompletionToken&& token) {
    boost::asio::async_initiate<CompletionToken, Signature>(detail::do_tick<CompletionToken, Signature>{}, token, interval, false, executor);
}
// 启动定时器并执行回调（一直重复，除非 io 停止）
template <class Executor, class CompletionToken, class Signature = void (boost::system::error_code)>
void tick(Executor& executor, std::chrono::steady_clock::duration interval, CompletionToken&& token) {
    boost::asio::async_initiate<CompletionToken, Signature>(detail::do_tick<CompletionToken, Signature>{}, token, interval, true, executor);
}
} // namespace time
} // namespace xbond
