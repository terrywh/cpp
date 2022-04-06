#pragma once
#include "../coroutine.hpp"
#include "mutex.hpp"
#include <vector>

namespace xbond {
namespace sync {
// 同步原语：协程信号量（简化）
class condition_variable {
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::vector< std::shared_ptr<coroutine> > co_;
 public:
    condition_variable(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io)) {}
    // 等待
    void wait(coroutine_handler& ch) {
        auto co = ch.co();
        boost::asio::post(strand_, [this, co] () {
            co_.push_back(co);
        });
        ch.yield(); // (1) <- resume
    }
    // 唤醒（一个等待中的协程）
    void notify_one() {
        boost::asio::post(strand_, [this] () {
            if (co_.empty()) return;
            (*co_.begin())->resume();
        });
    }
    // 唤醒（所有等待中的协程）
    void notify_all() {
        boost::asio::post(strand_, [this] () {
            for (auto i=co_.begin(); i!=co_.end(); i=co_.erase(i))
                (*i)->resume(); // resume -> (1)
        });
    }
};

} // namespace sync
} // namespace xbond
