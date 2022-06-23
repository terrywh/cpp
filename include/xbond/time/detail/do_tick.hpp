#pragma once
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/coroutine.hpp>

namespace xbond {
namespace time {
namespace detail {

template <class CompletionToken, class Signature>
struct do_tick : public boost::asio::coroutine {
    typedef BOOST_ASIO_HANDLER_TYPE(CompletionToken, Signature) handler_type;
    struct context {
        handler_type handler;
        std::chrono::steady_clock::duration duration;
        bool repeat = false;
        boost::asio::steady_timer timer;

        template <class... Args>
        context(handler_type& h, std::chrono::steady_clock::duration d, bool r, Args&&... args)
        : handler(h)
        , duration(d)
        , repeat(r)
        , timer(std::forward<Args>(args)...) {}
    };

    std::shared_ptr<context> context_;

    template <class DeduceHandler, class ...Args>
    void operator ()(DeduceHandler&& h, std::chrono::steady_clock::duration d, bool r, Args&&... args) {
        context_ = std::make_shared<context>(h, d, r, std::forward<Args>(args)...);
        (*this)({});
    }

    void operator ()(const boost::system::error_code& error) { BOOST_ASIO_CORO_REENTER(this) {
        do {
            context_->timer.expires_after(context_->duration);
            BOOST_ASIO_CORO_YIELD context_->timer.async_wait(std::move(*this));
            if (error) return;
            context_->handler(error);
        } while (context_->repeat);
    }}
};


} // namespace detail
} // namespace time
} // namespace xbond
