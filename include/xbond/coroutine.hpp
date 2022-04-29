#pragma once
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/context/fiber.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <system_error>
#include <variant> // 同时支持 std::error_code / boost::system::error_code 错误形态

namespace xbond {
// 协程处理器
template <class C>
class basic_coroutine_handler {
public:
    using coroutine_type = typename std::decay<C>::type;
    // 创建空的处理器
    basic_coroutine_handler() = default;
    // 创建指定协程的处理器
    explicit basic_coroutine_handler(std::shared_ptr<coroutine_type> co)
    : size_(nullptr)
    , err1_(nullptr)
    , err2_(nullptr)
    , co_(co) {}
    // 
    basic_coroutine_handler(const basic_coroutine_handler& ch) = default;
    // 指定错误返回（用于从异步流程带回错误）
    template <class ErrorT>
    basic_coroutine_handler& error(ErrorT& err) {
        if constexpr(std::is_same<boost::system::error_code, ErrorT>::value) err1_ = &err;
        if constexpr(std::is_same<std::error_code, ErrorT>::value) err2_ = &err;
        return *this;
    }
    template <class ErrorT>
    ErrorT* error() const {
        if constexpr(std::is_same<boost::system::error_code, ErrorT>::value) return err1_;
        if constexpr(std::is_same<std::error_code, ErrorT>::value) return err2_;
        return nullptr;
    }
    template <class ErrorT>
    basic_coroutine_handler& error(ErrorT* err) {
        if constexpr(std::is_same<boost::system::error_code, ErrorT>::value) err1_ = err;
        if constexpr(std::is_same<std::error_code, ErrorT>::value) return err2_ = err;
        return *this;
    }
    operator boost::system::error_code&() const { return *err1_; }
    operator std::error_code&() const { return *err2_; }
    // 指定错误返回（用于从异步流程带回错误）
    template <class ErrorT>
    basic_coroutine_handler& operator[](ErrorT& err) {
        return error(err);
    }
    // 模拟的回调
    template <class ErrorT, typename = typename std::enable_if<std::is_same<ErrorT, boost::system::error_code>::value || std::is_same<ErrorT, std::error_code>::value>::type>
    void operator()(const ErrorT& err, std::size_t count = 0) {
        if (size_) *size_ = count;
        resume(err);
    }
    // 重置处理器
    void reset() {
        size_ = nullptr;
        err1_ = nullptr;
        err2_ = nullptr;
        co_.reset();
    }
    // 重置处理器用于控制指定的协程
    void reset(std::shared_ptr<coroutine_type> co) {
        size_ = nullptr;
        err1_ = nullptr;
        err2_ = nullptr;
        co_ = co;
    }
    // 协程暂停
    void yield() {
        assert(co_);
        co_->yield(this);
    }
    // 协程暂停（用于从异步流程带回错误信息）
    template <class ErrorT>
    void yield(ErrorT& err) {    
        assert(co_);
        error(err);
        co_->yield(this);
    }
    // 协程恢复
    void resume() {
        assert(co_);
        co_->resume();
    }
    // 协程恢复（带回指定错误信息）
    template <class ErrorT>
    void resume(const ErrorT& err) {
        static_assert(std::is_same<ErrorT, boost::system::error_code>::value || std::is_same<ErrorT, std::error_code>::value, "only boost::system::error_code & std::error_code are supported");
        assert(co_);
        if constexpr (std::is_same<boost::system::error_code, ErrorT>::value) if (err1_) *err1_ = err;
        if constexpr (std::is_same<std::error_code, ErrorT>::value) if (err2_) *err2_ = err;
        co_->resume();
    }
    // 受控协程
    std::shared_ptr<coroutine_type> co() {
        return co_;
    }
    // 
    const boost::asio::strand<boost::asio::any_io_executor>& executor() const {
        return co_->executor();
    }

protected:
    std::size_t* size_; // 用于捕获长度数据
    boost::system::error_code* err1_;
    std::error_code* err2_; // 用于捕获错误值
    // 被管控的协程
    std::shared_ptr<coroutine_type> co_;

    friend coroutine_type;
    friend class boost::asio::async_result<
        basic_coroutine_handler<C>, void(boost::system::error_code err, std::size_t size)>;
    friend class boost::asio::async_result<
        basic_coroutine_handler<C>, void(boost::system::error_code err, std::size_t size)>;
    friend class boost::asio::async_result<
        basic_coroutine_handler<C>, void(std::error_code err, std::size_t size)>;
    friend class boost::asio::async_result<
        basic_coroutine_handler<C>, void(std::error_code err)>;
};
// 协程
class coroutine: public std::enable_shared_from_this<coroutine> {
protected:
    boost::context::fiber  c1_;
    boost::context::fiber  c2_;
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    boost::asio::executor_work_guard<boost::asio::any_io_executor> guard_;
    thread_local static std::weak_ptr<coroutine> current_;
public:
    static std::shared_ptr<coroutine> current() {
        return current_.lock();
    }
    // 注意：请使用 start 创建并启动协程
    template <class Executor>
    explicit coroutine(const Executor& ex)
    : strand_(ex)
    , guard_(ex) { }
    //
    const boost::asio::strand<boost::asio::any_io_executor>& executor() const {
        return strand_;
    }
    //
    void start() {}
    // 协程暂停
    void yield(void* handle = nullptr) {
        // current_ = nullptr;
        // 一般当前上下文位于当前执行器（线程）
        c2_ = std::move(c2_).resume();
    }
    // 协程恢复
    void resume() {
        // 恢复实际执行的上下文，可能需要对应执行器（线程）
        boost::asio::post(strand_, [this, self = this->shared_from_this()] () {
            current_ = self;
            c1_ = std::move(c1_).resume();
        });
    }
    // 
    void end() {}
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
private:
    // 启动协程
    template <class Executor, class Handler>
    static std::shared_ptr<coroutine> initiate(const Executor& executor, Handler&& fn) {
        auto co = std::make_shared<coroutine>(executor); 
        // 在执行器上运行协程
        boost::asio::post(co->executor(), [co, fn = std::move(fn)] () mutable {
            co->c1_ = boost::context::fiber(
            [co, fn = std::move(fn)] (boost::context::fiber&& c2) mutable {
                co->c2_ = std::move(c2);
                {
                    co->start();
                    current_ = co;
                    basic_coroutine_handler<coroutine> ch {co};
                    fn(ch); // 注意：协程函数若出现异常，应用程序会被立即结束
                    co->end();
                }
                return std::move(co->c2_);
            });
            co->c1_ = std::move(co->c1_).resume();
        });
        return co;
    }
};
using coroutine_handler = basic_coroutine_handler<coroutine>;

} // namespace xbond

namespace boost {
namespace asio {

template <>
class async_result<::xbond::coroutine_handler, 
    void(boost::system::error_code error, std::size_t size)> {
public:
    explicit async_result(::xbond::coroutine_handler& ch) : ch_(ch), size_(0) {
        ch_.size_ = &size_;
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

template <>
class async_result<::xbond::coroutine_handler, 
    void(std::error_code error, std::size_t size)> {
public:
    explicit async_result(::xbond::coroutine_handler& ch) : ch_(ch), size_(0) {
        ch_.size_ = &size_;
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
class async_result<::xbond::coroutine_handler, void(std::error_code error)> {
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
