#pragma once
#include "../coroutine.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace xbond {
namespace net {
/**
 * 判定提供的会话类型是否满足需求
 */
template <class T>
struct is_tcp_session {
    struct is_session_runable {
        template <typename U> static auto test(int) -> typename std::enable_if<
                std::is_invocable<decltype(&U::run), U&, coroutine_handler&>::value,
            std::true_type>::type;
        template <typename> static std::false_type test(...);
        constexpr static bool value = !std::is_same<decltype(test<T>(0)), std::false_type>::value;
    };
    struct is_session_startible {
        template <typename U> static auto test(int) -> typename std::enable_if<
                std::is_invocable<decltype(&U::start), U&>::value,
            std::true_type>::type;
        template <typename> static std::false_type test(...);
        constexpr static bool value = !std::is_same<decltype(test<T>(0)), std::false_type>::value;
    };

    template <typename U> static auto test(int) -> typename std::enable_if<
            std::is_constructible<U, boost::asio::ip::tcp::socket&&>::value && // 构造
           (is_session_runable::value || is_session_startible::value), // u.run(ch) || u.start()
        std::true_type>::type;
    template <typename> static std::false_type test(...);

    constexpr static bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template <class Session>
class tcp_server: public std::enable_shared_from_this<tcp_server<Session>> {
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket     socket_;

    void accept() {
        acceptor_.async_accept(socket_, 
        [this, self = this->shared_from_this()] (const boost::system::error_code& error) {
            if (error) return;
            if constexpr (is_tcp_session<Session>::is_session_runable::value)
                coroutine::start(socket_.get_executor(),
                [session = std::make_shared<Session>(std::move(socket_))] (coroutine_handler& ch) {
                    session->run(ch);
                });
            else if constexpr(is_tcp_session<Session>::is_session_startible::value)
                std::make_shared<Session>(std::move(socket_))->start();
            accept();
        });
    }

 public:
    tcp_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind)
    : acceptor_(io, bind, true)
    , socket_(io) {
        // boost::asio::ip::tcp::socket::reuse_address reuse_address(true);
        // acceptor_.set_option(reuse_address);
        // acceptor_.bind(bind);
    }
    void start() {
        acceptor_.listen();
        accept();
    }
    void close() {
        acceptor_.close();
        socket_.close();
    }
    boost::asio::ip::tcp::endpoint local_endpoint() {
        return acceptor_.local_endpoint();
    }
};

template <class Session, typename = typename std::enable_if<is_tcp_session<Session>::value, Session>::type>
std::shared_ptr<tcp_server<Session>> make_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind) {
    return std::make_shared<tcp_server<Session>>(io, bind);
}

} // namespace net
} // namespace xbond
