#pragma once
#include "../vendor.h"
#include "device.hpp"
#include "detail/address_storage.hpp"
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/ip/network_v6.hpp>

namespace xbond {
namespace net {
// 本地（内网）地址
class local_address {
    boost::asio::ip::address addr_;
public:
    // 遍历网络设备，获取第一个内网地址
    local_address() {
        foreach_device([this] (const device_info& device) {
            if (device.v4 && is_local_v4(device.v4.value())) {
                addr_ = boost::asio::ip::address_v4(device.v4.value());
                return false;
            }
            if (device.v6 && is_local_v6(device.v6.value())) {
                addr_ = boost::asio::ip::address_v6(device.v6.value());
                return false;
            }
            return true; // 继续遍历
        });
        if (addr_.is_unspecified())
            throw std::runtime_error("failed to retreive local address");
    }
    // 以 IPv4 地址构建
    local_address(boost::asio::ip::address_v4::bytes_type v4) {
        addr_ = boost::asio::ip::address_v4(v4);
    }
    // 以 IPv6 地址构建
    local_address(boost::asio::ip::address_v6::bytes_type v6) {
        addr_ = boost::asio::ip::address_v6(v6);
    }
    // 
    operator boost::asio::ip::address() const {
        return addr_;
    }
    // 返回地址的文本表达形式
    std::string str() const {
        return addr_.to_string();
    }
    friend std::ostream& operator <<(std::ostream& os, const local_address& addr) {
        return os << addr.addr_;
    }

 private:
    inline static boost::asio::ip::network_v4 n4a {boost::asio::ip::make_address_v4("10.0.0.0"), 8};
    inline static boost::asio::ip::network_v4 n4b {boost::asio::ip::make_address_v4("192.168.0.0"), 12};
    inline static boost::asio::ip::network_v4 n4c {boost::asio::ip::make_address_v4("172.16.0.0"), 16};
    inline static boost::asio::ip::network_v6 n6a {boost::asio::ip::make_address_v6("fd00::"), 8};
    inline static boost::asio::ip::network_v6 n6b {boost::asio::ip::make_address_v6("fc00::"), 8};
    // 
    static bool is_local_v4(const detail::ip_address_v4& addr) {
        boost::asio::ip::address_v4 v4(addr);
        boost::asio::ip::network_v4 n4(v4, 32);
        if (n4.is_subnet_of(n4a) || n4.is_subnet_of(n4b) || n4.is_subnet_of(n4c)) {
            return true;
        }
        return false;
    }
    // 
    static bool is_local_v6(const detail::ip_address_v6& addr) {
        boost::asio::ip::address_v6 v6(addr);
        boost::asio::ip::network_v6 n6(v6, 128);
        
        if (n6.is_subnet_of(n6a) || n6.is_subnet_of(n6b)) {
            return true;
        }
        return false;
    }
};

} // namespace net
} // namespace xbond
