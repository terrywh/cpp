#include <xbond/os/env.hpp>
using namespace xbond;

int os_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::cout << "\t" << os::executable() << std::endl;
}
