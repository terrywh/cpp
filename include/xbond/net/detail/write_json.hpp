#pragma once
#include <boost/json.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>

namespace xbond {
namespace net {
namespace detail {



template <class AsyncWriteStream, std::size_t BufferSize>
class write_json {
    // 用于包裹可移动的上下文
    struct context {
        AsyncWriteStream*            stream;
        boost::json::value*           value;
        boost::json::serializer  serializer;
        std::array<char, BufferSize> buffer;

        context(AsyncWriteStream& s, boost::json::value& v)
        : stream(&s)
        , value(&v) {
            serializer.reset(value);
        }
    };
    std::unique_ptr<context> ctx_;

 public:
    write_json(AsyncWriteStream& stream, boost::json::value& value) {
        ctx_ = std::make_unique<context>(stream, value);
    }

    template <class AsyncOperation>
    void operator()(AsyncOperation& self, boost::system::error_code error = {}, std::size_t size = 0) {
        if (error) return self.complete(error);
        if (ctx_->serializer.done()) return self.complete(error);

        auto sv = ctx_->serializer.read(ctx_->buffer.data(), ctx_->buffer.size());
        boost::asio::async_write(*ctx_->stream, boost::asio::buffer(sv.data(), sv.size()), std::move(self)); //
    }
};

} // namespace detail
} // namespace net
} // namespace xbond
