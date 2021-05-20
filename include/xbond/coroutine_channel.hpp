#pragma once
#include "vendor.h"
#include "coroutine.hpp"
#include <boost/lockfree/queue.hpp>

namespace xbond {
// 仿 Go 的 Channel 实现
// TODO: 注意要求 T 类型须满足: 1. 复制构造 2. 标准赋值 3. 标准销毁
template <class T, std::size_t Capacity>
class basic_coroutine_channel: public std::enable_shared_from_this<basic_coroutine_channel<T, Capacity>> {
    static_assert(std::is_copy_constructible<T>::value 
        && std::is_copy_assignable<T>::value 
        && std::is_destructible<T>::value,
        "T must be copy constructible, copy assignable and destructible");

    boost::asio::io_context::strand strand_; // 用于序列化对 into_ / from_ 容器的操作
    boost::lockfree::queue<T, boost::lockfree::capacity<Capacity>> queue_;
    std::set< std::shared_ptr<coroutine> > into_;
    std::set< std::shared_ptr<coroutine> > from_;
    int status_;
    enum {
        CLOSED = 0x01,
    };
public:
    basic_coroutine_channel(boost::asio::io_context& io)
    : strand_(io) {}

    // 向 CHANNEL 写入
    basic_coroutine_channel& into(const T& obj, coroutine_handler& ch) {
        while (!queue_.bounded_push(obj)) {
            boost::asio::post(strand_, [this, &ch] () {
                into_.emplace(ch.co());
            });
            ch.yield(); // (1) <- yield
        }
        boost::asio::post(strand_, [this, self = this->shared_from_this()] () {
            while (!from_.empty())
                from_.extract(from_.begin()).value()->resume(); // resume -> (2)
        });
        return *this;
    }
    // 从 CHANNEL 读取
    bool from(T& obj, coroutine_handler& ch) {
        while (!queue_.pop(obj)) {
            if (status_ & CLOSED) return false;

            boost::asio::post(strand_, [this, &ch] () {
                from_.emplace(ch.co());
            });
            ch.yield(); // (2) <- yield
        }
        boost::asio::post(strand_, [this, self = this->shared_from_this()] () {
            while (!into_.empty())
                into_.extract(into_.begin()).value()->resume(); // resume -> (1)
        });
        return true;
    }

    void close() {
        status_ |= CLOSED;
        boost::asio::post(strand_, [this, self = this->shared_from_this()] () {
            while (!from_.empty())
                from_.extract(from_.begin()).value()->resume(); // resume -> (2)
        });
    }
};

template <class T>
using unbuffered_channel = basic_coroutine_channel<T, 1ul>;

template <class T, std::size_t Capacity = 1ul>
std::shared_ptr<basic_coroutine_channel<T, Capacity>> make_channel(boost::asio::io_context& io) {
    return std::make_shared<basic_coroutine_channel<T, Capacity>>(io);
}

} // namespace xbond
