#pragma once
#include "../../coroutine.hpp"
#include "detail/server_handler.hpp"
#include <boost/beast/core/flat_static_buffer.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/mp11/list.hpp>

namespace xbond {
namespace net {
namespace http {

/**
 * 判定提供的处理器类型是否满足需求
 */
template <class T>
struct is_handler {
    template <typename U> static auto test(int) -> typename std::enable_if<
            std::is_constructible<U, boost::beast::tcp_stream&>::value/* && // 构造
           (std::is_invocable<decltype(&U::run), U&, coroutine_handler&>::value || // u.run(ch)
            std::is_invocable<decltype(&U::start), U&>::value)*/, // u.start()
        std::true_type>::type;
    template <typename> static std::false_type test(...);

    constexpr static bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template <class Body, std::size_t BufferSize = 16 * 1024>
class server {
    
    typedef typename std::decay<Body>::type       payload_type;
    typedef boost::beast::http::request<Body>     request;
    typedef boost::beast::http::response<Body>    response;
    boost::asio::io_context&         context_;
    boost::asio::ip::tcp::acceptor  acceptor_;
    
    template <class... Handler>
    class session {
        boost::beast::tcp_stream               stream_;
        boost::beast::flat_static_buffer<BufferSize> buffer_;
    public:
        template <class IOExecutor>
        session(IOExecutor ex)
        : stream_(ex) {}
        boost::asio::ip::tcp::socket& socket() {
            return stream_.socket();
        }
        void run(coroutine_handler& ch) {
            typedef boost::mp11::mp_list<Handler...> handler_list;
            boost::beast::http::parser<true, payload_type> parser;
            boost::beast::http::async_read_header(stream_, buffer_, parser, ch);
            boost::beast::http::async_read(stream_, buffer_, parser, ch);
            // hooks
            // on_header();
            request req = parser.get();
            response rsp;
            if constexpr(boost::mp11::mp_size<handler_list>::value > 0) {
                boost::mp11::mp_at_c<handler_list, 0> handler(stream_);
                handler(req, rsp, ch);
            }
            rsp.prepare_payload();
            boost::beast::http::async_write(stream_, rsp, ch);
            // on_finish();
        }
    };
public:
    server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind, bool reuse_port = false)
    : context_(io)
    , acceptor_(io, bind) {
#ifdef SO_REUSEPORT
        boost::asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> opt_reuse_port(reuse_port);
#endif
        acceptor_.set_option(opt_reuse_port);
    }
    template <class... Handler>
    void run(coroutine_handler& ch) {
        acceptor_.listen();
        while (true) { // TODO 结合 coroutine 提供 STOP 机制
            auto s = std::make_shared<session<Handler...>>(boost::asio::make_strand(context_));
            acceptor_.async_accept(s->socket(), ch);
            coroutine::start(s->socket().get_executor(), [s] (coroutine_handler ch) {
                s->run(ch);
            });
        }
    }
};

template <class Body, std::size_t BufferSize = 16 * 1024, class... Handler>
std::shared_ptr<server<Body, BufferSize>> make_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind) {
    return std::make_shared<server<Body, BufferSize>>(io, bind);
}

class server_handler_router {
    
};


} // namespace http
} // namespace net
} // namespace xbond
