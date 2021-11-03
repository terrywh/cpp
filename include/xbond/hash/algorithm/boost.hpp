#pragma once
#include "../../vendor.h"
#include <boost/crc.hpp>

namespace xbond {
namespace hash {
namespace algorithm {

#define CLASS_BOOST_CRC_ALGORITHM(NAME, A1, A2, A3, A4, A5, A6) class NAME {   \
 public:  \
    using algorithm_type = boost::crc_optimal<A1, A2, A3, A4, A5, A6>;         \
    using value_type = typename algorithm_type::value_type;                    \
 private: \
    algorithm_type crc_;                                                       \
 public:  \
    inline void update(const char* data, std::size_t size) {                   \
        crc_.process_bytes(data, size);                                        \
    }                                                                          \
    inline value_type digest() { return crc_.checksum(); }                     \
}

CLASS_BOOST_CRC_ALGORITHM(crc16, 16ul, 0x8005, 0, 0, true, true);
CLASS_BOOST_CRC_ALGORITHM(crc32, 32ul, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true);
CLASS_BOOST_CRC_ALGORITHM(crc64, 64ul, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, true, true);

} // namespace algorithm
} // namespace hash
} // namespace xbond
