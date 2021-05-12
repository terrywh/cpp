#pragma once
#include "../vendor.h"

namespace xbond {
namespace net {
/**
 * 简单封装地址信息，用于分离域名/地址和端口
 */
class address {
public:
    address()
    : host_(""), port_(0u) {}
    /**
     * @param addr 地址信息，例如 www.qq.com:443 或 127.0.0.1:8080
     */
    template <class StringView>
    explicit address(StringView addr) {
        std::size_t idx = addr.find_last_of(':');
        if (idx <= 1) throw std::runtime_error("failed to parse address: colon not found");

        host_.assign(addr.data(), idx);
        port_ = std::atoi(&addr[idx + 1]);
    }
    template <class StringView>
    address(StringView host, std::uint16_t port)
    : host_(host.data(), host.size()), port_(port) {}

    address(const address& addr) = default;
    // 域名/地址
    const std::string& host() const { return host_; }
    // 端口
    std::uint16_t port() const { return port_; }
    // 服务（端口）
    std::string service() const {
        std::string str;
        str.resize(6);
        std::size_t len = std::sprintf(const_cast<char*>(str.data()), "%u", port_);
        str.resize(len);
        return str;
    }
    std::string str() const {
        std::string str;
        str.resize(host_.size() + 8);
        std::size_t len = std::sprintf(const_cast<char*>(str.data()), "%s:%u", host_.c_str(), port_);
        str.resize(len);
        return str;
    }
    operator boost::asio::ip::tcp::endpoint() const {
        return boost::asio::ip::tcp::endpoint{ boost::asio::ip::make_address(host_), port_ };
    }
    operator boost::asio::ip::udp::endpoint() const {
        return boost::asio::ip::udp::endpoint{ boost::asio::ip::make_address(host_), port_ };
    }
    bool operator ==(const address& addr) const {
        return port_ == addr.port_ && host_ == addr.host_;
    }
    bool operator !=(const address& addr) const {
        return port_ != addr.port_ || host_ != addr.host_;
    }
private:
    std::string   host_;
    std::uint16_t port_;
};

} // namespace net
} // namespace xbond
