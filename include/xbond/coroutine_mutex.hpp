#pragma once
#include "vendor.h"
#include "coroutine.hpp"

namespace xbond {
// 同步原语：协程信号量
class coroutine_unique_mutex {
 protected:
    boost::asio::strand<boost::asio::io_context::executor_type> strand_; // 序列化对 co_ 容器的操作
    int                                unique_;
    std::set< std::shared_ptr<coroutine> > co_;
 public:
    coroutine_unique_mutex(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io))
    , unique_(0) {}
    
    void lock(coroutine_handler& ch) {
        boost::asio::post(strand_, [this, &ch] () {
            if (++unique_ == 1) ch.resume();
            else co_.insert(ch.co()); // 等待锁的协程
        });
        ch.yield();
    }

    void unlock(coroutine_handler& ch) {
        assert(unique_ > 0);
        boost::asio::post(strand_, [this, &ch] () {
            --unique_;
            if (!co_.empty()) // 恢复一个等待的协程
                co_.extract(co_.begin()).value()->resume();
            ch.resume();
        });
        ch.yield();
    }
};
// 协程信号量
class coroutine_shared_mutex {
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    int                                unique_;
    int                                shared_;
    std::set< std::shared_ptr<coroutine> > co_;
    std::set< std::shared_ptr<coroutine> > so_;
 public:
    coroutine_shared_mutex(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io))
    , unique_(0)
    , shared_(0) { }

    void lock(coroutine_handler& ch) {
        boost::asio::post(strand_, [this, &ch] () {
            if (++shared_ == 1 && ++unique_ == 1) ch.resume();
            else co_.insert(ch.co()); // 等待锁的协程
        });
        ch.yield();
    }

    void unlock(coroutine_handler& ch) {
        assert(unique_ > 0);
        boost::asio::post(strand_, [this, &ch] () {
            --unique_;
            --shared_;
            // 最后一个独占锁结束，尝试其他独占、共享锁
            if (!co_.empty())
                co_.extract(co_.begin()).value()->resume();
            else while (!so_.empty())
                so_.extract(co_.begin()).value()->resume();
            ch.resume();
        });
        ch.yield();
    }

    void shared_lock(coroutine_handler& ch) {
        boost::asio::post(strand_, [this, &ch] () {
            ++shared_;            
            if (unique_ == 0) ch.resume();
            else so_.insert(ch.co()); // 等待锁的协程
        });
        ch.yield();
    }

    void shared_unlock(coroutine_handler& ch) {
        assert(shared_ > 0);
        boost::asio::post(strand_, [this, &ch] () {
            // 最后一个共享锁释放，尝试是否存在独占锁
            if (--shared_ == 0 && !co_.empty())
                co_.extract(co_.begin()).value()->resume();
            ch.resume();
        });
        ch.yield();
    }
};
}