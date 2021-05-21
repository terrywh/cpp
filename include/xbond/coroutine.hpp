#pragma once
#include "vendor.h"
#include <variant>

namespace xbond {
// 
class basic_coroutine_error_proxy {
    std::variant<std::error_code*, boost::system::error_code*> v_;
 public:
    basic_coroutine_error_proxy(std::variant<std::error_code*, boost::system::error_code*> v)
    : v_(v) {}
    operator std::error_code&() { return *std::get<std::error_code*>(v_); }
    operator boost::system::error_code&() { return *std::get<boost::system::error_code*>(v_); }
};
// 协程处理器
template <class C>
class basic_coroutine_handler {
public:
    using coroutine_type = typename std::decay<C>::type;
    // 创建空的处理器
    basic_coroutine_handler()
    : count_(nullptr) {}
    // 创建指定协程的处理器
    explicit basic_coroutine_handler(std::shared_ptr<coroutine_type> co)
    : count_(nullptr)
    , co_(co) {}
    // 
    basic_coroutine_handler(const basic_coroutine_handler& ch) = default;
    // 指定错误引用（用于从异步流程带回错误）
    basic_coroutine_handler& operator[](boost::system::error_code& error) {
        error_ = &error;
        return *this;
    }
    // 指定错误返回（用于从异步流程带回错误）
    basic_coroutine_handler& operator[](std::error_code& error) {
        error_ = &error;
        return *this;
    }
    // 模拟的回调
    void operator()(const boost::system::error_code& error, std::size_t count = 0) {
        if (error_) *std::get<boost::system::error_code*>(error_.value()) = error;
        if (count_) *count_ = count;
        resume();
    }
    // 
    const boost::asio::strand<boost::asio::any_io_executor>& executor() const {
        return co_->executor();
    }
    // 重置处理器
    void reset() {
        count_ = nullptr;
        error_.reset();
        co_.reset();
    }
    // 重置处理器用于控制指定的协程
    void reset(std::shared_ptr<coroutine_type> co) {
        count_ = nullptr;
        error_.reset();
        co_ = co;
    }
    // 协程暂停
    inline void yield() {
        co_->yield();
    }
    // 协程暂停（用于从异步流程带回错误信息）
    inline void yield(boost::system::error_code& error) {
        error_ = &error;
        co_->yield();
    }
    // 协程暂停（用于从异步流程带回错误信息）
    inline void yield(std::error_code& error) {
        error_ = &error;
        co_->yield();
    }
    // 协程恢复
    inline void resume() {
        co_->resume();
    }
    // 协程恢复（带回指定错误信息）
    inline void resume(const boost::system::error_code& error) {
        if (error_) *std::get<boost::system::error_code*>(error_.value()) = error;
        co_->resume();
    }
    // 协程恢复（带回指定错误信息）
    inline void resume(const std::error_code& error) {
        if (error_) *std::get<std::error_code*>(error_.value()) = error;
        co_->resume();
    }

    inline std::shared_ptr<coroutine_type> co() {
        return co_;
    }
    // 获取当前持有的错误信息
    basic_coroutine_error_proxy error() {
        if (!error_) throw std::runtime_error("error not set");
        return {error_.value()};
    }

protected:
    std::size_t* count_; // 用于捕获长度数据
    std::optional< std::variant<std::error_code*, boost::system::error_code*> > error_; // 用于捕获错误值
    // 被管控的协程
    std::shared_ptr<coroutine_type> co_;

    friend coroutine_type;
    friend class boost::asio::async_result<
        basic_coroutine_handler<C>, void(boost::system::error_code error, std::size_t size)>;
};
// 协程
template <class Hook>
class basic_coroutine: public std::enable_shared_from_this<basic_coroutine<Hook>> {
protected:
    using hook_type = typename std::decay<Hook>::type;
    hook_type              co_;
    boost::context::fiber  c1_;
    boost::context::fiber  c2_;
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    boost::asio::executor_work_guard<boost::asio::any_io_executor> guard_;

    thread_local static basic_coroutine* current_;
public:
    static std::shared_ptr<basic_coroutine> current() {
        return current_->shared_from_this();
    }

    // 注意：请使用 go 创建并启动协程
    template <class Executor>
    explicit basic_coroutine(const Executor& ex)
    : strand_(ex)
    , guard_(ex) { }
    //
    const boost::asio::strand<boost::asio::any_io_executor>& executor() const {
        return strand_;
    }
    //
    void start() {
        co_.start();
    }
    // 协程暂停
    void yield() {
        co_.yield();
        // current_ = nullptr;
        // 一般当前上下文位于当前执行器（线程）
        c2_ = std::move(c2_).resume();
    }
    // 协程恢复
    void resume() {
        // 恢复实际执行的上下文，可能需要对应执行器（线程）
        boost::asio::post(strand_, [this, self = this->shared_from_this()] () {
            co_.resume();
            current_ = this;
            c1_ = std::move(c1_).resume();
        });
    }
    // 
    void end() {
        co_.end();
    }
    // 启动协程
    template <class Executor, class Handler>
    static void start(const Executor& executor, Handler&& fn, typename std::enable_if<
            boost::asio::is_executor<Executor>::value || boost::asio::execution::is_executor<Executor>::value
        >::type* v = nullptr) {
        initiate(executor, fn);
    }
    // 启动协程
    template <class ExecutionContext, class Handler>
    static void start(ExecutionContext& ctx, Handler&& fn, typename std::enable_if<
            std::is_convertible<ExecutionContext*, boost::asio::execution_context*>::value
        >::type* v = nullptr) {
        initiate(ctx.get_executor(), fn);
    }
    // 使协程休眠一段时间
    static void sleep(std::chrono::steady_clock::duration duration, basic_coroutine_handler<basic_coroutine<Hook>>& ch) {
        boost::asio::steady_timer timer(ch.executor());
        timer.expires_after(duration);
        timer.async_wait(ch);
    }
    // 使“当前”协程休眠一段时间
    static void sleep(std::chrono::steady_clock::duration duration) {
        basic_coroutine_handler<basic_coroutine<Hook>> ch{current()};
        sleep(duration, ch);
    }

private:
    // 启动协程
    template <class Executor, class Handler>
    static void initiate(const Executor& executor, Handler&& fn) {
        auto co = std::make_shared<basic_coroutine<Hook>>(executor); 
        // 在执行器上运行协程
        boost::asio::post(co->executor(), [co, fn = std::move(fn)] () mutable {
            co->c1_ = boost::context::fiber(
            [co, fn = std::move(fn)] (boost::context::fiber&& c2) mutable {
                co->c2_ = std::move(c2);
                {
                    co->start();
                    current_ = co.get();
                    basic_coroutine_handler<basic_coroutine<Hook>> ch {co};
                    fn(ch); // 注意：协程函数若出现异常，应用程序会被立即结束
                    co->end();
                }
                return std::move(co->c2_);
            });
            co->c1_ = std::move(co->c1_).resume();
        });
    }
    // template <class T>
    // friend class basic_coroutine_handler;
};

template <class Hook>
thread_local basic_coroutine<Hook>* basic_coroutine<Hook>::current_;

class basic_coroutine_hook {
public:
    basic_coroutine_hook() = default;
    void  start() {}
    void  yield() {}
    void resume() {}
    void    end() {}
};

using coroutine = basic_coroutine<basic_coroutine_hook>;
using coroutine_handler = basic_coroutine_handler<coroutine>;

} // namespace xbond

namespace boost {
namespace asio {

template <>
class async_result<::xbond::coroutine_handler, 
    void(boost::system::error_code error, std::size_t size)> {
public:
    explicit async_result(::xbond::coroutine_handler& ch) : ch_(ch), size_(0) {
        ch_.count_ = &size_;
    }
    using completion_handler_type = ::xbond::coroutine_handler;
    using return_type = std::size_t;
    return_type get() {
        ch_.yield();
        return size_;
    }
private:
    ::xbond::coroutine_handler &ch_;
    std::size_t size_;
};

template <>
class async_result<::xbond::coroutine_handler, void(boost::system::error_code error)> {
public:
    explicit async_result(::xbond::coroutine_handler& ch) : ch_(ch) {
    }
    using completion_handler_type = ::xbond::coroutine_handler;
    using return_type = void;
    void get() {
        ch_.yield();
    }
private:
    ::xbond::coroutine_handler &ch_;
};

} // namespace asio
} // namespace boost
