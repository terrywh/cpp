
#include <xbond/hash/crc16.hpp>
#include <xbond/hash/crc32.hpp>
#include <xbond/hash/crc64.hpp>
#include <xbond/hash/sha1.hpp>
#include <xbond/hash/sha256.hpp>
#include <xbond/encoding/hex.hpp>
#include <iostream>
using namespace xbond;

int hash_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string o = "中文";
    std::cout << "\tcrc16(" << o << ") -> " << hash::crc16::hash(o) << std::endl;
    std::cout << "\tcrc32(" << o << ") -> " << hash::crc32::hash(o) << std::endl;
    std::cout << "\tcrc64(" << o << ") -> " << hash::crc64::hash(o) << std::endl;
    std::cout << "\tsha1(" << o << ") -> " << encoding::hex::encode(hash::sha1::hash(o)) << std::endl;
    std::cout << "\tsha256(" << o << ") -> " << encoding::hex::encode(hash::sha256::hash(o)) << std::endl;
    return 0;
}