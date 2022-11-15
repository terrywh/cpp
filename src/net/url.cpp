#include <xbond/net/url.hpp>
#include <xbond/strconv/parse_string.hpp>

namespace xbond {
namespace net {

const net::url& empty_url() {
    static net::url empty;
    return empty;
}

static const std::string_view pattern {":/@?#", 5};

void assign(std::string_view& target, std::string_view source, std::size_t f, std::size_t e) {
    if (e != f) target = source.substr(f, e - f);
}

url parse_url(std::string_view str, bool copy) {
    url u;
    std::size_t f = 0, e = 0;
    enum expect : int {
        scheme = 0x01,
        user   = 0x02,
        pass   = 0x04,
        domain = 0x08,
        port   = 0x10,
        path   = 0x20,
        query  = 0x40,
        hash   = 0x80,
    };
    int x = scheme | user | domain;

    while (f < str.size() - 1) {
        e = str.find_first_of(pattern, f);

        if (x & expect::scheme) {
            if (e != str.npos && e + 2 < str.size() && str[e] == ':' && str[e+1] == '/' && str[e+2] == '/') {
                u.scheme = str.substr(f, e-f);
                x &= ~expect::scheme;
                x |=  expect::user | expect::domain;

                e += 2;
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::user) {
            if (e != str.npos && str[e] == ':') {
                u.username = str.substr(f, e-f);
                x &= ~(expect::user | expect::scheme);
                x |=  expect::pass;
                // continue; // 不存在认证区段，可能直接到达域名区段
            }
            else if (e != str.npos && str[e] == '@') {
                u.username = str.substr(f, e-f);
                x &= ~(expect::user | expect::pass);
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::pass) {
            if (e != str.npos && str[e] == '@') {
                u.password = str.substr(f, e-f);
                u.domain = {};
                x &= ~(expect::scheme | expect::user | expect::pass);
                x |=  expect::domain;
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::domain) {
            if (e == str.npos && f < str.size() || str[e] == ':' || str[e] == '/') {
                u.domain = str.substr(f, e-f);
                x &= ~(expect::scheme | expect::user | expect::domain); // 有可能错误的将用户名存储为域名，后续可能遇到密码
                x |=  expect::port | expect::path | expect::query | expect::hash;
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::port) {
            if (str[f-1] == ':' && (e == str.npos || str[e] == '/' || str[e] == '?' || str[e] == '#')) {
                u.port = str.substr(f, e-f);
                x &= ~expect::port;
                x |=  expect::path | expect::query | expect::hash;
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::path) {
            if (e == str.npos && f < str.size() || str[e] == '?' || str[e] == '#') {
                u.path = str.substr(f-1, e-f+1);
                x &= ~(expect::port | expect::path);
                x |=  expect::query | expect::hash;
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::query) {
            if (e == str.npos && f < str.size() || str[e] == '#') {
                u.query = str.substr(f-1, e-f+1);
                x &= ~(expect::port | expect::path | expect::query);
                x |=  expect::hash;
                goto CONTINUE_PARSING;
            }
        }
        if (x & expect::hash) {
            if (e == str.npos && f < str.size()) {
                u.hash = str.substr(f-1, e-f+1);
                x &= ~(expect::path | expect::query | expect::hash);
                goto CONTINUE_PARSING;
            }
        }
    CONTINUE_PARSING:
        if (e == str.npos) break;
        f = e + 1;
    }
    if (!u.username.empty() && u.password.empty() && u.username.data() == u.domain.data())
        u.username = {};
    if (copy) u.raw = str;
    return u;
}



#define COPY_STRING_VIEW(FIELD) FIELD(raw.data() + (u.FIELD.data() - u.raw.data()), u.FIELD.size())
url::url(const url& u)
: raw(u.raw)
, COPY_STRING_VIEW(scheme)
, COPY_STRING_VIEW(username)
, COPY_STRING_VIEW(password)
, COPY_STRING_VIEW(domain)
, COPY_STRING_VIEW(port)
, COPY_STRING_VIEW(path)
, COPY_STRING_VIEW(query)
, COPY_STRING_VIEW(hash) {

}
#undef COPY_STRING_VIEW

url::operator xbond::net::address() const {
    return { domain, strconv::parse_string(port) };
}
#define ASSIGN_STRING_VIEW(FIELD) FIELD = std::string_view(raw.data() + (u.FIELD.data() - u.raw.data()), u.FIELD.size())
url& url::operator =(const url& u) {
    raw = u.raw;
    ASSIGN_STRING_VIEW(scheme);
    ASSIGN_STRING_VIEW(username);
    ASSIGN_STRING_VIEW(password);
    ASSIGN_STRING_VIEW(domain);
    ASSIGN_STRING_VIEW(port);
    ASSIGN_STRING_VIEW(path);
    ASSIGN_STRING_VIEW(query);
    ASSIGN_STRING_VIEW(hash);
    return *this;
}
#undef ASSIGN_STRING_VIEW

std::ostream& operator<<(std::ostream& os, const url& u) {
    if (u.raw.empty()) {
        if (!u.scheme.empty()) os << u.scheme << "://";
        if (!u.username.empty() && !u.password.empty()) os << u.username << ":" << u.password << "@";
        else if (!u.username.empty()) os << u.username << "@";
        os << u.domain;
        if (!u.port.empty()) os << ":" << u.port;
        os << u.path << u.query << u.hash;
    }
    else os << u.raw;
    return os;
}

} // namespace net
} // namespace xbond
