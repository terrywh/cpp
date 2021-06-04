#pragma once
#include "../vendor.h"
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>

namespace xbond {
namespace encoding {

template <class DynamicBuffer_v1>
class dynamic_buffer_sink_v1: public boost::iostreams::sink {
    DynamicBuffer_v1& buffer_;
 public:
    dynamic_buffer_sink_v1(DynamicBuffer_v1& v1)
    : buffer_(v1) {}
    std::streamsize write(const char* data, std::streamsize size) {
        auto buffers = buffer_.prepare(size);
        const char* offset = data;
        std::size_t remain = size;
        for (auto i=boost::asio::buffer_sequence_begin(buffers); 
            remain > 0 && i!=boost::asio::buffer_sequence_end(buffers); ++i) {
            boost::asio::mutable_buffer mb = *i;
            std::size_t chunk_size = mb.size() < remain ? mb.size() : remain;
            std::memcpy(mb.data(), offset, chunk_size);
            offset += chunk_size;
            remain -= chunk_size;
        }
        return size - remain;
    }
};

}
}