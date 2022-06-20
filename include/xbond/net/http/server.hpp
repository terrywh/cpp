#pragma once
#include "../../coroutine.hpp"
#include "../server.hpp"
#include "null_body.hpp"
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/log/trivial.hpp>
#include <map>
#include <memory>
#include <string>

namespace xbond {
namespace net {
namespace http {

using server_parser = boost::beast::http::request_parser<boost::beast::http::empty_body>;
template <class Buffer = boost::beast::flat_static_buffer<4096>>
class server : public std::enable_shared_from_this<server<Buffer>> {
public:
    using  handler_type = std::function<void (boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& req, coroutine_handler& ch)>;
    
private:
    boost::asio::io_context&                   context_;
    boost::asio::ip::tcp::acceptor            acceptor_;
    boost::asio::ip::tcp::endpoint             address_;
    std::map<boost::string_view, handler_type> handler_;

public:
    server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind, bool reuse_port = false)
    :  context_(io)
    , acceptor_(io)
    ,  address_(bind) {
        acceptor_.open(address_.protocol());
        boost::asio::socket_base::reuse_address opt_reuse_addr {true};
        acceptor_.set_option(opt_reuse_addr);
#ifdef SO_REUSEPORT
        boost::asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> opt_reuse_port(reuse_port);
        acceptor_.set_option(opt_reuse_port);
#endif
        handle(":status:404", [] (boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& parser, coroutine_handler& ch) {
            boost::beast::http::response<boost::beast::http::empty_body> not_found(boost::beast::http::status::not_found, 11);
            not_found.keep_alive(parser.get().keep_alive());
            not_found.prepare_payload();
            boost::beast::http::async_write(stream, not_found, ch);
        });
        handle(":status:500", [] (boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& parser, coroutine_handler& ch) {
            boost::beast::http::response<boost::beast::http::empty_body> internal_server_error(boost::beast::http::status::internal_server_error, 11);
            internal_server_error.keep_alive(parser.get().keep_alive());
            internal_server_error.prepare_payload();
            boost::beast::http::async_write(stream, internal_server_error, ch);
        });
        
    }
    server(const server&) = delete;
    server(server&&) = delete;
    virtual ~server() = default;
    void run(coroutine_handler& ch) {
        boost::system::error_code error, *origin = ch.error<boost::system::error_code>();
        acceptor_.bind(address_, error);
        if (error) goto RETURN_ERROR;
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);
        while (!error) {
            auto stream = std::make_shared<boost::beast::tcp_stream>(context_);
            acceptor_.async_accept(stream->socket(), ch[error]);
            if (error) break;
            coroutine::start(context_, [this, stream, self = this->shared_from_this()] (xbond::coroutine_handler& ch) {
                run_handler(stream, ch);
            });
        }
        RETURN_ERROR:
        if (error && origin) *origin = error;
        ch.error(origin);
    }

    /**
     * @param path 调用者需保证 path 指向的字符串在服务器执行期间有效
     */
    void handle(boost::string_view path, handler_type handler) {  handler_[path] = handler; }

    virtual void before_handle(boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& parser, coroutine_handler& ch) {}
    virtual void after_handle(boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& parser, coroutine_handler& ch) {}

    void close() {
        boost::system::error_code error;
        acceptor_.close(error);
    }

private:
    void run_handler(std::shared_ptr<boost::beast::tcp_stream> stream, xbond::coroutine_handler& ch) {
        boost::system::error_code error;
        Buffer buffer;
        while (true) {
            std::unique_ptr<server_parser> parser = std::make_unique<server_parser>();
            boost::beast::http::async_read_header(*stream, buffer, *parser, ch[error]);
            if (error) break;

            before_handle(*stream, buffer, *parser, ch[error]);
            
            if (!run_handler(path(parser->get().target()), *stream, buffer, *parser, ch[error]))
                run_handler(":status:404", *stream, buffer, *parser, ch[error]);
            if (error) run_handler(":status:500", *stream, buffer, *parser, ch[error]);

            after_handle(*stream, buffer, *parser, ch[error]);
            
            if (error || parser->get().target().empty()) continue;
            if (!parser->is_done()) { // 将 handle 未处理的 body 进行忽略读取后继续下次请求
                boost::beast::http::request_parser<null_body> file { std::move(*parser) };
                boost::beast::http::async_read(*stream, buffer, file, ch[error]);
            }
            if (error) break;
        }
        stream->socket().close(error);
    }

    boost::string_view path(boost::string_view uri) {
        if (std::size_t i = uri.find_first_of('?'); i == uri.npos) return uri;
        else return uri.substr(0, i);
    } 

    bool run_handler(boost::string_view path, boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& req, coroutine_handler& ch) {
        if (auto i=handler_.find(path); i!=handler_.end()) { i->second(stream, buffer, req, ch); return true; }
        else return false;
    }
};

template <class Buffer = boost::beast::flat_static_buffer<4096UL>>
std::shared_ptr<server<Buffer>> make_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind, bool reuse_port = false) {
    return std::make_shared<server<Buffer>>(io, bind, reuse_port);
}

} // namespace http
} // namespace net
} // namespace xbond
