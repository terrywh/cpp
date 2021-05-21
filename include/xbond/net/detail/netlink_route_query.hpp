#pragma once
#include "../../vendor.h"
#include "../device_info.hpp"
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>

namespace xbond {
namespace net {
namespace detail {

class netlink_route_query_attr {
    const char* data_;
    std::size_t size_;

    netlink_route_query_attr(void* data, std::size_t size)
    : data_(static_cast<const char*>(data)), size_(size) {}

public:
    const char* data() { return data_; };
    std::size_t size() { return size_; }
    friend class netlink_route;
};

struct netlink_route_query_get_addr {
    constexpr std::uint16_t type() { return RTM_GETADDR; }
    using payload_type = struct ifaddrmsg;
    using attribute_type = netlink_route_query_attr;
    void setup(payload_type* payload) {
        payload->ifa_family = AF_PACKET;
    }
    int parse(payload_type* payload) {
        return payload->ifa_index;
    }
    void parse(unsigned short type, attribute_type attr, device_info& info) {
        if (type == IFA_LABEL) {
            info.name = std::string{attr.data(), std::min(attr.size(), std::strlen(attr.data()))};
        }
        else if (type == IFA_ADDRESS) {
            if (attr.size() > 10) {
                ip_address_v6 v6;
                std::memcpy(v6.data(), attr.data(), 16);
                info.v6 = v6;
            }
            else {
                ip_address_v4 v4;
                std::memcpy(v4.data(), attr.data(), 4);
                info.v4 = v4;
            }
        }
    }
};

struct netlink_route_query_get_link {
    constexpr std::uint16_t type() { return RTM_GETLINK; }
    using payload_type = struct ifinfomsg;
    using attribute_type = netlink_route_query_attr;
    void setup(payload_type* payload) {
        payload->ifi_family = AF_PACKET;
        payload->ifi_change = 0xffffffff; // according to man docs
    }
    int parse(payload_type* payload) {
        return payload->ifi_index;
    }
    void parse(unsigned short type, attribute_type attr, device_info& info) {
        if (type == IFLA_IFNAME)
            info.name = std::string{attr.data(), std::min(attr.size(), std::strlen(attr.data()))};
        else if (type == IFLA_ADDRESS)
            std::memcpy(info.hw.data(), attr.data(), 6);
    }
};



} // namespace detail
} // namespace net
} // namespace xbond
