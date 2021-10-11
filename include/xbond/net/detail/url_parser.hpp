#pragma once
#include <boost/spirit/include/qi.hpp>
#include <string>

namespace xbond {
namespace net {
namespace detail {

// 提供简单的默认端口覆盖机制
class url_port {
    int port_ = 0;
    
 public:
    url_port(): port_(0) {}
    url_port(int port): port_(port) {}
    url_port& operator = (const url_port& up) {
        if (up.port_ != 0) port_ = up.port_;
        return *this;
    }
    operator int() const { return port_; }
    template <class Iterator>
    friend class url_parser;
};

typedef struct url_s {
    std::string scheme;
    std::string user;
    std::string password;
    std::string domain;
    int         port;
    std::string path;  // 含起始 "/" 符号
    std::string query; // 含起始 "?" 符号
    std::string hash;  // 含起始 "#" 符号
} url_t;

template <class Iterator>
struct url_parser: boost::spirit::qi::grammar<Iterator, url_t()> {

    template <class T>
    struct output {
        T& field_;
        explicit output(T& field)
        : field_(field) {}
        void operator ()(T v, boost::spirit::qi::unused_type, boost::spirit::qi::unused_type) const {
            field_ = v;
        }
    };

    url_t& u;
    url_parser(url_t& url)
    : url_parser::base_type(start, "url")
    , u(url) {
        scheme = +(boost::spirit::ascii::char_ - ':' - '/');
        user = +(boost::spirit::ascii::char_ - ':' - '@');
        password = +(boost::spirit::ascii::char_ - '@');
        domain = +(boost::spirit::ascii::char_ - ':' - '/');
        port = boost::spirit::qi::int_;
        path = boost::spirit::ascii::char_('/') > +(boost::spirit::ascii::char_ - '?' - '#');
        query = boost::spirit::ascii::char_('?') > +(boost::spirit::ascii::char_ - '#');
        hash = boost::spirit::ascii::char_('#') > +boost::spirit::ascii::char_;
        start = -(scheme[output<std::string>(u.scheme)] >> "://")
            >> -(user[output<std::string>(u.user)] >> -(':' >> password[output<std::string>(u.password)]) >> '@')
            >> domain[output<std::string>(u.domain)]
            >> -(':' >> port[output<int>(u.port)])
            >> -path[output<std::string>(u.path)]
            >> -query[output<std::string>(u.query)]
            >> -hash[output<std::string>(u.hash)];
        
        start.name("start");
        scheme.name("scheme");
        user.name("user");
        password.name("password");
        domain.name("domain");
        port.name("port");
        path.name("path");
        query.name("query");
        hash.name("hash");

        // boost::spirit::qi::on_error(start, [] (auto args, auto context, bool r) {
        //     std::cout 
        //         << "expected " << boost::fusion::at_c<3>(args).tag << ": " << boost::fusion::at_c<3>(args)
        //         << " got " << std::string_view {boost::fusion::at_c<0>(args), boost::fusion::at_c<1>(args) - boost::fusion::at_c<0>(args)} << std::endl;
        // });
    }
    boost::spirit::qi::rule<Iterator, url_t()> start;
    boost::spirit::qi::rule<Iterator, std::string()> scheme;
    boost::spirit::qi::rule<Iterator, std::string()> user;
    boost::spirit::qi::rule<Iterator, std::string()> password;
    boost::spirit::qi::rule<Iterator, std::string()> domain;
    boost::spirit::qi::rule<Iterator, int()> port;
    boost::spirit::qi::rule<Iterator, std::string()> path;
    boost::spirit::qi::rule<Iterator, std::string()> query;
    boost::spirit::qi::rule<Iterator, std::string()> hash;
};


} // namespace detail
} // namespace net
} // namespace xbond

