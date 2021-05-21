#include <iostream>

#define LOGGER() std::cout << "\t\t"

extern int net_http_client_test(int argc, char* argv[]);

int net_http_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    net_http_client_test(argc, argv);
}