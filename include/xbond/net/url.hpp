#pragma once
#include "address.hpp"
#include "detail/url_parser.hpp"
#include "../detail/to_string_view.hpp"

namespace xbond {
namespace net {

/// 封装对标准形式的 URL 对象，提供基本的解析功能，并提供以下数据属性访问：
/// @property std::string scheme   - 协议头
/// @property std::string user     - 账号
/// @property std::string password - 密码
/// @property std::string domain   - 域名
/// @property int         port     - 端口
/// @property std::string path     - 路径，含起始 "/" 符号
/// @property std::string query    - 查询，含起始 "?" 符号
/// @property std::string hash     - 哈希，含起始 "#" 符号
class url: public detail::url_t {
    bool empty_;

 public:
    // 构建一个“空”状态的 URL 对象
    url(int port = 0)
    : empty_(true) {
        this->port = port;
    }
    /// 解析 URL 字符串构建 URL 对象
    /// @throws boost::spirit::qi::expectation_failure<std::string::iterator>
    ///    expect {ex.what_} got {ex.fisrt ~ ex.last}
    template <class S, typename = typename std::enable_if<xbond::detail::convertible_to_string_view<S>::value, S>::type>
    url(const S& str, std::uint16_t port = 0)
    : empty_(true) {
        this->port = port; 

        std::string_view sv = xbond::detail::to_string_view(str);
        if (sv.empty()) return;
        bool r = false;
        r = boost::spirit::qi::parse(sv.begin(), sv.end(), detail::url_parser<std::string_view::const_iterator>(*this));
        // 按照目前的实现在发生解析错误时会抛出异常，故当解析失败时，函数并未返回 r == false
        if (r) {
            // 由于解析可能发生回溯，用户名密码可能包含错误数据
            std::string_view domain_ = domain;
            std::string_view user_ = {user.data(), std::min(domain.size(), user.size())};
            if (domain_ == user_) {
                user.clear();
                password.clear();
            }
            empty_ = false;
        }
    }
    // 
    void assign(const url& url) {
        operator=(url);
    }
    operator xbond::net::address() const {
        return { domain, port };
    }
    url& operator =(const url& url) = default;
    
    bool empty() const { return empty_; }
    operator bool() const { return empty_; }
    // 重新构一个对应 URL 文本
    std::string str() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }
    std::string svc() const {
        return std::to_string(port);
    }
    // 
    friend std::ostream& operator<<(std::ostream& os, const url& u) {
        os << u.scheme << "://";
        if (!u.user.empty()) {
            os << u.user;
            if (!u.password.empty()) os << ':' << u.password;
            os << '@';
        }
        os << u.domain;
        if (u.port) os << ":" << u.port;
        os << u.path << u.query << u.hash;
        return os;
    }
};

static const url& empty_url();

} // namespace net
} // namespace xbond
