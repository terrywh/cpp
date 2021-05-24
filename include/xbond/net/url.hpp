#pragma once
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
    bool parsed_;

 public:
    // 构建一个“空”状态的 URL 对象
    url(int port = 0)
    : parsed_(true) {
        this->port = port;
    }
    /// 解析 URL 字符串构建 URL 对象
    /// @throws boost::spirit::qi::expectation_failure<std::string::iterator>
    ///    expect {ex.what_} got {ex.fisrt ~ ex.last}
    template <class S, typename = typename std::enable_if<xbond::detail::to_string_view_invokable<S>::value, S>::type>
    url(S str, int port = 0)
    : parsed_(false) {
        this->port = port; 

        std::string_view sv = xbond::detail::to_string_view(str);
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
            parsed_ = true;
        }
    }
    // 
    void assign(const url& url) {
        operator=(url);
    }
    url& operator =(const url& url) = default;
    // 若发生了解析过程（如使用了复制或从字符串构建），确认解析结果
    operator bool() const { return parsed_; };
    // 重新构一个对应 URL 文本
    std::string str() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
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

} // namespace net
} // namespace xbond
