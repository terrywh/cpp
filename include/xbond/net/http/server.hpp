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
    std::map<boost::string_view, handler_type> handler_;
    handler_type handler_notfound_;

public:
    server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind, bool reuse_port = false)
    :  context_(io)
    , acceptor_(io, bind) {
#ifdef SO_REUSEPORT
        boost::asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> opt_reuse_port(reuse_port);
        acceptor_.set_option(opt_reuse_port);
#endif
        handler_notfound_ = [] (boost::beast::tcp_stream& stream, Buffer& buffer, server_parser& parser, coroutine_handler& ch) {
            // boost::beast::http::request_parser<boost::beast::http::string_body> request { std::move(parser) };
            // boost::beast::http::async_read(stream, buffer, request, ch);

            boost::beast::http::response<boost::beast::http::empty_body> not_found(boost::beast::http::status::not_found, 11);
            not_found.keep_alive(parser.get().keep_alive());
            not_found.prepare_payload();
            boost::beast::http::async_write(stream, not_found, ch);
        };
    }
    server(const server&) = delete;
    server(server&&) = delete;
    virtual ~server() = default;
    void run(coroutine_handler& ch) {
        boost::system::error_code error, *origin = ch.error<boost::system::error_code>();
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);
        while (!error) {
            auto stream = std::make_shared<boost::beast::tcp_stream>(context_);
            acceptor_.async_accept(stream->socket(), ch[error]);
            if (error) break;
            coroutine::start(context_, 
                [this, stream, server = this->shared_from_this()] (coroutine_handler& ch) mutable {
                boost::system::error_code error;
                Buffer buffer;
                while (true) {
                    std::unique_ptr<server_parser> parser = std::make_unique<server_parser>();
                    boost::beast::http::async_read_header(*stream, buffer, *parser, ch[error]);
                    if (error) break;

                    before_handle(*stream, buffer, *parser, ch[error]);

                    if (auto i = handler_.find(parser->get().target()); i != handler_.end()) i->second(*stream, buffer, *parser, ch[error]);
                    else handler_notfound_(*stream, buffer, *parser, ch[error]); // 未匹配的 404 响应

                    after_handle(*stream, buffer, *parser, ch[error]);
                    // 将 handle 未处理的 body 进行忽略读取后继续下次请求
                    if (error || parser->get().target().empty()) continue;
                    if (!parser->is_done()) {
                        boost::beast::http::request_parser<null_body> file { std::move(*parser) };
                        boost::beast::http::async_read(*stream, buffer, file, ch[error]);
                    }
                    if (error) break;
                }
                stream->socket().close(error);
            });
        }
        if (error && origin) *origin = error;
        ch.error(origin);
    }

    void not_found(handler_type handler) { handler_notfound_ = handler; }
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
};

template <class Buffer = boost::beast::flat_static_buffer<4096UL>>
std::shared_ptr<server<Buffer>> make_server(boost::asio::io_context& io, boost::asio::ip::tcp::endpoint bind, bool reuse_port = false) {
    return std::make_shared<server<Buffer>>(io, bind, reuse_port);
}

} // namespace http
} // namespace net
} // namespace xbond
