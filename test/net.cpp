#include <xbond/net/address.hpp>
#include <xbond/net/device.hpp>
#include <xbond/net/hardware_address.hpp>
#include <xbond/net/local_address.hpp>
#include <xbond/net/url.hpp>
#include <xbond/encoding/hex.hpp>
#include <iostream>
using namespace xbond;

#define LOGGER() std::cout

extern int net_client_test(int argc, char* argv[]);
extern int net_tcp_server_test(int argc, char* argv[]);
extern int net_http_test(int argc, char* argv[]);

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
}

int net_hardware_address_test(int argc, char* argv[]) {
    LOGGER() << "\t" << __func__ << "\n";
    LOGGER() << "\t\t" << net::hardware_address() << "\n";
    return 0;
}

int net_url_test(int argc, char* argv[]) {
    LOGGER() << "\t" << __func__<< "\n";
    net::url url, url2(443);
    url = "https://user:pass@www.qq.com:443/path?query#hash"; // true
    url2 = net::url("https://user@www.qq.com/path#hash", 443); // true
    net::url url3("http://www.qq.com?query"); // true
    net::url url4("http://www.qq.com"); // true
    net::url url5("www.qq.com"); // false

    LOGGER() << "\t\tscheme = " << url.scheme << "\n";
    LOGGER() << "\t\tuser = " << url.user << "\n";
    LOGGER() << "\t\tpassword = " << url.password << "\n";
    LOGGER() << "\t\tdomain = " << url.domain << "\n";
    LOGGER() << "\t\tport = " << url.port << "\n";
    LOGGER() << "\t\tport = " << url2.port << "\n";
    LOGGER() << "\t\tpath = " << url.path << "\n";
    LOGGER() << "\t\tquery = " << url.query << "\n";
    LOGGER() << "\t\thash = " << url.hash << "\n";
    LOGGER() << "\t\turl2 = " << !!url2 << "\n";
    LOGGER() << "\t\turl3 = " << !!url3 << "\n";
    LOGGER() << "\t\turl4 = " << !!url4 << "\n";
    LOGGER() << "\t\turl5 = " << !!url5 << "\n";

    LOGGER() << "\t\turl = " << url << "\n";
    LOGGER() << "\t\turl3 = " << url3 << "\n";
}

int net_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    net::address addr("www.qq.com:443");
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";
    addr = "1.1.1.1:2222";
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";
    addr = "[2:2:2::2]:3333";
    LOGGER() << "\t" << addr << " / " << addr.service() << "\n";

    net_device_test(argc, argv);
    net_hardware_address_test(argc, argv);
    net_url_test(argc, argv);
    net_client_test(argc, argv);
    net_tcp_server_test(argc, argv);
    net_http_test(argc, argv);

    return 0;
}