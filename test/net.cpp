#include <xbond/net/address.hpp>
#include <xbond/net/device.hpp>
#include <xbond/net/hardware_address.hpp>
#include <xbond/net/local_address.hpp>
#include <xbond/net/url.hpp>
#include <xbond/net/parse_query.hpp>
#include <xbond/encoding/hex.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout

extern int net_url_test(int argc, char* argv[]);
extern int net_client_test(int argc, char* argv[]);
extern int net_tcp_server_test(int argc, char* argv[]);
extern int net_http_test(int argc, char* argv[]);

int net_address_test(int argc, char* argv[]) {
    net::address addr("www.qq.com:443");
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";
    addr = "1.1.1.1:2222";
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";
    addr = "[2:2:2::2]:3333";
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";
    addr = net::address{"106.53.137.138:8730"};
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";
    return 0;
}

int net_url_test(int argc, char* argv[]) {
    LOGGER() << "\t" << __func__<< "\n";
    auto u1 = net::parse_url("https://user:pass@www.qq.com:443/path?query#hash");
    LOGGER() << "\t\t" << u1 << "\n";
    auto u2 = net::parse_url("user:pass@www.qq.com");
    LOGGER() << "\t\t" << u2 << "\n";
    auto u3 = net::parse_url("https://www.qq.com/path");
    LOGGER() << "\t\t" << u3 << "\n";
    auto u4 = net::parse_url("www.qq.com/path");
    LOGGER() << "\t\t" << u4 << "\n";
    auto u5 = net::parse_url("user@www.qq.com");
    LOGGER() << "\t\t" << u5 << "\n";
    auto u6 = net::parse_url("https://user@www.qq.com/path?query#hash", true);
    LOGGER() << "\t\t" << u6 << "\n";
    auto u7 = net::parse_url("user:pass", true);
    LOGGER() << "\t\t" << u7 << "\n";
    auto u8 = net::parse_url("", true);
    LOGGER() << "\t\t" << u8 << "\n";
    return 0;
}

int net_parse_query_test(int argc, char* argv[]) {
    LOGGER() << "\t" << __func__ << "\n";
    const char* url = "http://blog.terrywh.net/search?p1=12%2F34&p2=%E4%B8%AD%E5%9B%BD%E4%BA%BA";
    net::url u = net::parse_url(url);
    LOGGER() << u.query << "\n";
    net::parse_query(u.query, [] (std::pair<std::string, std::string>&& entry) {
        std::pair<std::string, std::string> e { std::move(entry) };
        LOGGER() << e.first << " => " << e.second << "\n";
    });
    return 0;
}

int net_device_test(int argc, char* argv[]) {
    LOGGER() << "\t" << __func__ << "\n";
    net::foreach_device([] (const net::device_info& device) {
        LOGGER() << "\t\t" << device.idx << " -> " << encoding::hex::encode(device.hw);
        if (device.v4) 
            LOGGER()  << " v4: " << encoding::hex::encode(device.v4.value());
        if (device.v6)
            LOGGER()  << " v6: " << encoding::hex::encode(device.v6.value());
        LOGGER()  << "\n";
        return true;
    });
    return 0;
}

int net_hardware_address_test(int argc, char* argv[]) {
    LOGGER() << "\t" << __func__ << "\n";
    LOGGER() << "\t\t" << net::hardware_address() << "\n";
    return 0;
}

int net_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    net_address_test(argc, argv);
    net_device_test(argc, argv);
    net_hardware_address_test(argc, argv);
    net_url_test(argc, argv);
    net_parse_query_test(argc, argv);
    net_client_test(argc, argv);
    net_tcp_server_test(argc, argv);
    net_http_test(argc, argv);

    return 0;
}