#pragma once
#include "coroutine.hpp"
#include <boost/lockfree/queue.hpp>
#include <set>

namespace xbond {
// 仿 Go 的 Channel 实现
// TODO: 注意要求 T 类型须满足: 1. 复制构造 2. 标准赋值 3. 标准销毁
template <class T, std::size_t Capacity>
class basic_coroutine_channel {
    static_assert(std::is_copy_constructible<T>::value 
        && std::is_copy_assignable<T>::value 
        && std::is_destructible<T>::value,
        "T must be copy constructible, copy assignable and destructible");

    boost::asio::strand<boost::asio::io_context::executor_type>   strand_; // 用于序列化对 into_ / from_ 容器的操作
    boost::lockfree::queue<T, boost::lockfree::capacity<Capacity>> queue_;
    std::set< std::shared_ptr<coroutine> > into_;
    std::set< std::shared_ptr<coroutine> > from_;
    int status_;
    enum {
        CLOSED = 0x01,
    };
public:
    basic_coroutine_channel(boost::asio::io_context& io)
    : strand_(boost::asio::make_strand(io))
    , status_(0) {}

    // 向 CHANNEL 写入（可能“阻塞”指定协程）
    basic_coroutine_channel& into(const T& obj, coroutine_handler& ch) {
        while (!queue_.bounded_push(obj)) {
            boost::asio::post(strand_, [this, &ch] () {
                into_.insert(ch.co());
            });
            ch.yield(); // (1) <- yield
        }
        boost::asio::post(strand_, [this] () {
            while (!from_.empty()) {
                from_.extract(from_.begin()).value()->resume(); // resume -> (2)
            }
        });
        return *this;
    }
    // 向 CHANNEL 写入（可能“阻塞”当前协程）
    basic_coroutine_channel& operator << (const T& obj) {
        coroutine_handler ch{coroutine::current()};
        return into(obj, ch);
    }
    // 从 CHANNEL 读取（可能“阻塞”指定协程）
    bool from(T& obj, coroutine_handler& ch) {
        while (!queue_.pop(obj)) {
            if (status_ & CLOSED) return false;

            boost::asio::post(strand_, [this, &ch] () {
                from_.insert(ch.co());
            });
            ch.yield(); // (2) <- yield
        }
        boost::asio::post(strand_, [this] () {
            while (!into_.empty()) {
                into_.extract(into_.begin()).value()->resume(); // resume -> (1)
            }
        });
        return true;
    }
    // 从 CHANNEL 读取（可能“阻塞”当前协程）
    bool operator >> (T& obj) {
        coroutine_handler ch{coroutine::current()};
        return from(obj, ch);
    }
    // 关闭
    void close() {
        status_ |= CLOSED;
        boost::asio::post(strand_, [this] () {
            while (!from_.empty()) {
                from_.extract(from_.begin()).value()->resume(); // resume -> (2)
            }
        });
    }
};

template <class T>
using unbuffered_channel = basic_coroutine_channel<T, 1ul>;
template <class T, std::size_t N>
using channel = basic_coroutine_channel<T, N>;

} // namespace xbond
