#pragma once
#include "vendor.h"
#include "coroutine.hpp"
#include "coroutine_mutex.hpp"

namespace xbond {
// 同步原语：信号量（简化）
class coroutine_condition_variable {
    boost::asio::io_context::strand    strand_;
    std::set< std::shared_ptr<coroutine> > co_;
 public:
    coroutine_condition_variable(boost::asio::io_context& io)
    : strand_(io) {}
    // 等待
    void wait(coroutine_handler& ch) {
        auto co = ch.co();
        boost::asio::post(strand_, [this, co] () {
            co_.insert(co);
        });
        ch.yield(); // (1) <- resume
        boost::asio::post(strand_, [this, co] () {
            co_.extract(co);
            co->resume(); // (2) <- resume
        });
        ch.yield(); // (2) <- resume
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
            for (auto i=co_.begin(); i!=co_.end(); ++i)
                (*i)->resume(); // resume -> (1)
            // 由于实际 resume() 后进行 EXTRACT 的操作也 POST 到 strand_ 进行，
            // 故在做 resume() 时不会立即对 co_ 容器进行操作（不会造成竞争问题）
        });
    }
};

}