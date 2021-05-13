#pragma once
#include <boost/optional.hpp>
#include <boost/json.hpp>
#include <boost/beast/http.hpp>
#include <array>

namespace xbond {
namespace net {
namespace http {
// 配合 Boost.JSON 及 Boost.Beast 库，接收和发送 JSON 形式的 HTTP 消息
template <class StaticBuffer = std::array<char, 4096>>
class json_body {
public:
    using value_type = boost::json::value;
    // BUFFER -> JSON
    struct reader {
        template <bool isRequest, class Fields>
        reader(boost::beast::http::header<isRequest, Fields>& header, value_type& body)
        : body_(body)
        , read_(0) {}
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
                wrote += parser_.write(
                    static_cast<const char*>(buffer.data()), buffer.size(), error);
            }
            read_ += wrote;
            return wrote;
        }
        //
        void finish(boost::system::error_code& error) {
            error = {};
            if (read_ > 0) {
                try {
                    body_ = parser_.release();
                } catch(const boost::system::system_error& ex) {
                    error = ex.code();
                }
            }
        }
    private:
        value_type&           body_;
        boost::json::parser parser_;
        std::uint32_t         read_;
    };
    // JSON -> BUFFER
    struct writer {
        using const_buffers_type = boost::asio::const_buffer;
        //
        template <bool isRequest, class Fields>
        writer(boost::beast::http::header<isRequest, Fields>& header, value_type& value)
        : value_(value) {}
        //
        void init(boost::system::error_code& error) {
            error = {};
            serializer_.reset(&value_);
        }
        //
        boost::optional<std::pair<const_buffers_type, bool>> get(boost::system::error_code& error) {
            error = {};
            for (bool done = serializer_.done(); !done; ) {
                auto actual = serializer_.read(buffer_.data(), buffer_.size());
                return {{{actual.data(), actual.size()}, true}};
            }
            return boost::none;
        }

    private:
        boost::json::value&          value_;
        boost::json::serializer serializer_;
        StaticBuffer                buffer_;
    };
};

} // namespace http
} // namespace net
} // namespace xbond
