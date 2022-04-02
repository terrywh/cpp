
#include <xbond/hash/hash.hpp>
#include <xbond/encoding/hex.hpp>
#include <iostream>
using namespace xbond;

int hash_test(int argc, char* argv[]) {
    hash::algorithm::crc16::value_type v = 1;

    std::cout << __func__ << "\n";
    std::string o = "中文";
    std::cout << "\tcrc16(" << o << ") -> " << hash::crc16::digest(o) << std::endl;
    std::cout << "\tcrc32(" << o << ") -> " << hash::crc32::digest(o) << std::endl;
    std::cout << "\tcrc64(" << o << ") -> " << hash::crc64::digest(o) << std::endl;
    std::cout << "\tmd5(" << o << ") -> " << encoding::hex::encode(hash::md5::digest(o)) << std::endl;
    std::cout << "\tsha1(" << o << ") -> " << encoding::hex::encode(hash::sha1::digest(o)) << std::endl;
    std::cout << "\tsha256(" << o << ") -> " << encoding::hex::encode(hash::sha256::digest(o)) << std::endl;
    std::cout << "\tsha512(" << o << ") -> " << encoding::hex::encode(hash::sha512::digest(o)) << std::endl;
    return 0;
}