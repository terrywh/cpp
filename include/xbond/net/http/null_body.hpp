#pragma once
#include <boost/optional.hpp>
#include <boost/json.hpp>
#include <boost/beast/http.hpp>
#include <array>

namespace xbond {
namespace net {
namespace http {
// 配合 Boost.JSON 及 Boost.Beast 库，接收和发送 JSON 形式的 HTTP 消息
class null_body {
public:
    using value_type = std::nullptr_t;
    // BUFFER -> NULL
    struct reader {
        template <bool isRequest, class Fields>
        reader(boost::beast::http::header<isRequest, Fields>& header, value_type& body) {}
        //
        void init(boost::optional<std::uint64_t> content_length, boost::system::error_code& error) {
            boost::ignore_unused(content_length);
            error = {};
        }
        // 
        template <class ConstBufferSequence>
        std::size_t put(ConstBufferSequence const& buffers, boost::system::error_code& error) {
            error = {};
            std::size_t wrote = 0;
            for(auto i=boost::asio::buffer_sequence_begin(buffers),
                    e = boost::asio::buffer_sequence_end(buffers); i!=e; ++i) {
                boost::asio::const_buffer buffer {*i};
                wrote += buffer.size();
            }
            return wrote;
        }
        //
        void finish(boost::system::error_code& error) {
            error = {};
        }
    };
    // NULL -> BUFFER
    struct writer {
        using const_buffers_type = boost::asio::const_buffer;
        //
        template <bool isRequest, class Fields>
        writer(boost::beast::http::header<isRequest, Fields>& header, value_type& value) {}
        //
        void init(boost::system::error_code& error) {
            error = {};
        }
        //
        boost::optional<std::pair<const_buffers_type, bool>> get(boost::system::error_code& error) {
            error = {};
            return boost::none;
        }
    };
};

} // namespace http
} // namespace net
} // namespace xbond
