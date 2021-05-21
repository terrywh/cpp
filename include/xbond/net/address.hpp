#pragma once
#include "../vendor.h"

namespace xbond {
namespace net {
/**
 * 简单封装地址信息，用于分离域名/地址和端口
 */
class address {
    std::string   host_;
    std::uint16_t port_;

public:
    address()
    : host_(""), port_(0u) {}
    /**
     * 分割 ":" IP 地址与端口
     * @param addr 地址信息，例如 www.qq.com:443 或 127.0.0.1:8080 或 [ff::127::1]:8080 或 ff::127::1:8080
     * 注意，这里允许 IPv6 地址不使用 [] 包裹，并使用最后一个 ":" 分割地址与端口
     */
    template <class StringView, typename = typename std::enable_if<std::is_convertible<StringView, std::string_view>::value, StringView>::type>
    explicit address(StringView addr) {
        std::string_view sv = addr;
        std::size_t idx = sv.find_last_of(':');
        if (idx == sv.npos) throw std::runtime_error("failed to parse: colon not found");
        // 忽略 IPv6 的包裹括号
        if (sv[0] == '[') host_.assign(sv.data() + 1, idx - 2);
        else host_.assign(sv.data(), idx);
        port_ = std::atoi(&sv[idx + 1]);
    }
    /**
     * 指定域名或IP地址及端口构建完整地址信息
     */
    template <class StringView, typename = typename std::enable_if<std::is_convertible<StringView, std::string_view>::value, StringView>::type>
    address(StringView host, std::uint16_t port)
    : port_(port) {
        std::string_view sv = host;
        host_.assign(sv.data(), sv.size());
    }

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
    // 复制并返回完整的地址
    std::string str() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
    // 重新赋值
    template <class StringView, typename = typename std::enable_if<std::is_convertible<StringView, std::string_view>::value, StringView>::type>
    address& operator =(StringView s) {
        address addr {s};
        host_ = addr.host_;
        port_ = addr.port_;
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
    // 实现 Comparable 可比较
    bool operator <(const address& addr) const {
        if (host_ < addr.host_) return true;
        if (port_ < addr.port_) return true;
        return false;
    }
    //
    friend std::ostream& operator<<(std::ostream& os, const address& addr) {
        std::string_view sv = addr.host_;
        // 含有 ":" 按照 IPv6 地址处理
        if (sv.find_first_of(':') != sv.npos) return os << "[" << addr.host_ << "]:" << addr.port_;
        else return os << "" << addr.host_ << ":" << addr.port_;
    }
};

} // namespace net
} // namespace xbond
