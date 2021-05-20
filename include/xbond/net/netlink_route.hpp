#pragma once
#include "../vendor.h"
#include "../type_traits.hpp"
#include "detail/netlink_address.hpp"
#include <boost/process/environment.hpp>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <iostream>

namespace xbond {
namespace net {

class netlink_route_query_get_link;
class netlink_route_query_get_addr;

class netlink_route {
 public:
    struct device {
        int             idx;
        std::string    name;
        hw_address       hw;
        std::optional<ip_address_v4> v4;
        std::optional<ip_address_v6> v6;
    };

    class attribute {
        const char* data_;
        std::size_t size_;

        attribute(void* data, std::size_t size)
        : data_(static_cast<const char*>(data)), size_(size) {}

    public:
        const char* data() { return data_; };
        std::size_t size() { return size_; }
        friend class netlink_route;
    };

 private:
    static unsigned int seq_;
    unsigned int pid_;
    struct sockaddr_nl sa_;
    int fd_;
    char buffer_[8192];

    template <class RouteQuery>
    void execute_query(std::map<int, netlink_route::device>& device) {
        std::memset(buffer_, 0, sizeof(buffer_));

        RouteQuery query;
        struct iovec  iov;
        struct msghdr msg;
        int  len;
        bool end = false;
        // 消息头：需要计算携带 Payload 的大小
        struct nlmsghdr* nh = reinterpret_cast<struct nlmsghdr*>(buffer_);
        nh->nlmsg_len = NLMSG_LENGTH(sizeof(typename RouteQuery::payload_type));
        nh->nlmsg_type = query.type();
        nh->nlmsg_pid = pid_;
        nh->nlmsg_seq = ++seq_;
        // 请求+遍历所有
        nh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
        query.setup(reinterpret_cast<typename RouteQuery::payload_type*>(NLMSG_DATA(nh)));
        // 发送请求
        iov = { nh, nh->nlmsg_len };
        msg = { &sa_, sizeof(sa_), &iov, 1, nullptr, 0ul, 0 };
        len = sendmsg(fd_, &msg, 0);
        assert(len >= 0);
        // 接收
        iov = { buffer_, sizeof(buffer_) };
        msg = { &sa_, sizeof(sa_), &iov, 1, nullptr, 0ul, 0 };
        while (!end) {
            len = recvmsg(fd_, &msg, 0);
            assert(len >= 0);
            nh = reinterpret_cast<struct nlmsghdr*>(buffer_);
            // 解析此次接收到的数据
            while (NLMSG_OK(nh, len)) {
                if (nh->nlmsg_type == NLMSG_DONE || nh->nlmsg_type == NLMSG_ERROR) {
                    end = true;
                    nh = NLMSG_NEXT(nh, len);
                    continue;
                }
                int index = query.parse(reinterpret_cast<typename RouteQuery::payload_type*>(NLMSG_DATA(nh)));
                auto& dev = device[index];
                dev.idx = index;
                auto* attr = reinterpret_cast<struct rtattr*>(
                    reinterpret_cast<char*>(NLMSG_DATA(nh)) +
                    NLMSG_ALIGN(sizeof(typename RouteQuery::payload_type)));
                std::size_t size = NLMSG_PAYLOAD(nh, sizeof(typename RouteQuery::payload_type));
                // 解析附加属性
                while (RTA_OK(attr, size)) {
                    query.parse(attr->rta_type, {RTA_DATA(attr), attr->rta_len}, dev);
                    attr = RTA_NEXT(attr, size);
                }
                nh = NLMSG_NEXT(nh, len);
            }
        }
    }
    
 public:

    netlink_route() {
        fd_ = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
        struct sockaddr_nl sa;
        std::memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;
        std::memset(&sa_, 0, sizeof(sa));
        sa_.nl_family = AF_NETLINK;
        int r = bind(fd_, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa));
        assert(r >= 0);
        socklen_t size;
        // 使用系统分配的 PID (端口标识 PortId 可能与当前进程 PID 一致)
        getsockname(fd_, reinterpret_cast<struct sockaddr*>(&sa), &size);
        pid_ = sa.nl_pid;
    }
    ~netlink_route() {
        int r = close(fd_);
        assert(r >= 0);
    }
    // 遍历设备信息（回调存在返回值）
    template <class Handler>
    void foreach_device(Handler&& cb) {
        std::map<int, netlink_route::device> device;
        // 执行查询
        execute_query<netlink_route_query_get_link>(device);
        execute_query<netlink_route_query_get_addr>(device);
        // 
        for (auto i=device.begin(); i!=device.end(); ++i) 
            if (!static_cast<bool>(cb(i->second))) break;
    }
};


struct netlink_route_query_get_addr {
    constexpr std::uint16_t type() { return RTM_GETADDR; }
    using payload_type = struct ifaddrmsg;
    using attribute_type = netlink_route::attribute;
    void setup(payload_type* payload) {
        payload->ifa_family = AF_PACKET;
    }
    int parse(payload_type* payload) {
        return payload->ifa_index;
    }
    void parse(unsigned short type, attribute_type attr, netlink_route::device& info) {
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
    using attribute_type = netlink_route::attribute;
    void setup(payload_type* payload) {
        payload->ifi_family = AF_PACKET;
        payload->ifi_change = 0xffffffff; // according to man docs
    }
    int parse(payload_type* payload) {
        return payload->ifi_index;
    }
    void parse(unsigned short type, attribute_type attr, netlink_route::device& info) {
        if (type == IFLA_IFNAME)
            info.name = std::string{attr.data(), std::min(attr.size(), std::strlen(attr.data()))};
        else if (type == IFLA_ADDRESS)
            std::memcpy(info.hw.data(), attr.data(), 6);
    }
};


} // namespace net
} // namespace xbond
