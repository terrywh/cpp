#include <xbond/net/address.hpp>
#include <xbond/net/hardware_address.hpp>
#include <xbond/net/local_address.hpp>
#include <xbond/encoding/hex.hpp>
#include <iostream>
using namespace xbond;

extern int net_tcp_server_test(int argc, char* argv[]); 
extern int test_net_http_client(int argc, char* argv[]);

int net_netlink_route_list_device_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    net::netlink_route nr;
    nr.foreach_device([] (const net::netlink_route::device& device) {
        std::cout << "\t\t" << device.idx << " -> " << encoding::hex::encode(device.hw);
        if (device.v4) 
            std::cout << " v4: " << encoding::hex::encode(device.v4.value());
        if (device.v6)
            std::cout << " v6: " << encoding::hex::encode(device.v6.value());
        std::cout << "\n";
        return true;
    });
}

int net_hardware_address_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::cout << "\t\t" << net::hardware_address() << std::endl;
    return 0;
}

int net_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    net::address addr("1.1.1.1:2222");
    std::cout << "\t" << addr.host() << ":" << addr.port() << "/" << addr.service() << std::endl;
    addr = "2.2.2.2:3333";
    std::cout << "\t" << addr.host() << ":" << addr.port() << "/" << addr.service() << std::endl;
    std::cout << "\t";
    net_netlink_route_list_device_test(argc, argv);
    std::cout << "\t";
    net_hardware_address_test(argc, argv);
    std::cout << "\t";
    net_tcp_server_test(argc, argv);
    std::cout << "\t";
    test_net_http_client(argc, argv);
}