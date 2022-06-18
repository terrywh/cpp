#pragma once
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/async_result.hpp>

namespace xbond {
namespace time {

template <class CompletionToken, class Timer = boost::asio::steady_timer>
struct timer_context {
    template <class Executor>
    timer_context(Executor& executor, CompletionToken&& token)
    : timer(executor), completion (token) {}
    Timer timer;
    boost::asio::async_completion<CompletionToken, void(boost::system::error_code)> completion;
};


} // namespace time
} // namespace xbond
