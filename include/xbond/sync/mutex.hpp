#pragma once
#include "../coroutine.hpp"
#include <vector>

namespace xbond {
namespace sync {
// 同步原语：协程信号量
class unique_mutex {
 protected:
    boost::asio::strand<boost::asio::io_context::executor_type> strand_; // 序列化对 co_ 容器的操作
    int                                unique_;
    std::vector< std::shared_ptr<coroutine> > co_;
 public:
    unique_mutex(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io))
    , unique_(0) {}
    
    void lock(coroutine_handler& ch) {
        boost::asio::post(strand_, [this, &ch] () {
            if (++unique_ == 1) ch.resume();
            else co_.push_back(ch.co()); // 等待锁的协程
        });
        ch.yield();
    }

    void unlock(coroutine_handler& ch) {
        assert(unique_ > 0);
        boost::asio::post(strand_, [this, &ch] () {
            --unique_;
            if (!co_.empty()) { // 恢复一个等待的协程
                auto co = co_.front();
                co_.erase(co_.begin());
                co->resume();
            }
            ch.resume();
        });
        ch.yield();
    }
};
// 协程信号量
class shared_mutex {
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    int                                unique_;
    int                                shared_;
    std::vector< std::shared_ptr<coroutine> > co_;
    std::vector< std::shared_ptr<coroutine> > so_;
 public:
    shared_mutex(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io))
    , unique_(0)
    , shared_(0) { }

    void lock(coroutine_handler& ch) {
        boost::asio::post(strand_, [this, &ch] () {
            if (++shared_ == 1 && ++unique_ == 1) ch.resume();
            else co_.push_back(ch.co()); // 等待锁的协程
        });
        ch.yield();
    }

    void unlock(coroutine_handler& ch) {
        assert(unique_ > 0);
        boost::asio::post(strand_, [this, &ch] () {
            --unique_;
            --shared_;
            // 最后一个独占锁结束，尝试其他独占、共享锁
            while (!so_.empty()) {
                auto co = so_.front();
                co_.erase(so_.begin());
                co->resume();
            }
            if (!co_.empty()) {
                auto co = co_.front();
                co_.erase(co_.begin());
                co->resume();
            }
            ch.resume();
        });
        ch.yield();
    }

    void shared_lock(coroutine_handler& ch) {
        boost::asio::post(strand_, [this, &ch] () {
            ++shared_;            
            if (unique_ == 0) ch.resume();
            else so_.push_back(ch.co()); // 等待锁的协程
        });
        ch.yield();
    }

    void shared_unlock(coroutine_handler& ch) {
        assert(shared_ > 0);
        boost::asio::post(strand_, [this, &ch] () {
            // 最后一个共享锁释放，尝试是否存在独占锁
            if (--shared_ == 0 && !co_.empty()) {
                auto co = co_.front();
                co_.erase(co_.begin());
                co->resume();
            }
            ch.resume();
        });
        ch.yield();
    }
};

} // namespace sync
} // namespace xbond
