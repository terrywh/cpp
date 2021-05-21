#include <xbond/net/address.hpp>
#include <xbond/net/device.hpp>
#include <xbond/net/hardware_address.hpp>
#include <xbond/net/local_address.hpp>
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
            std::cout << " v4: " << encoding::hex::encode(device.v4.value());
        if (device.v6)
            std::cout << " v6: " << encoding::hex::encode(device.v6.value());
        std::cout << "\n";
        return true;
    });
}

int net_hardware_address_test(int argc, char* argv[]) {
    std::cout << "\t" << __func__ << "\n";
    std::cout << "\t\t" << net::hardware_address() << "\n";
    return 0;
}

int net_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    net::address addr("1.1.1.1:2222");
    LOGGER() << "\t" << addr.host() << ":" << addr.port() << "/" << addr.service() << "\n";
    addr = "2.2.2.2:3333";
    LOGGER() << "\t" << addr.host() << ":" << addr.port() << "/" << addr.service() << "\n";

    net_device_test(argc, argv);
    net_hardware_address_test(argc, argv);
    net_client_test(argc, argv);
    net_tcp_server_test(argc, argv);
    net_http_test(argc, argv);

    return 0;
}