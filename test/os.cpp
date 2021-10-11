#include <xbond/os/env.hpp>
#include <xbond/os/unique_file_lock.hpp>
using namespace xbond;

int os_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::cout << "\t" << os::executable() << std::endl;
    {
        os::unique_file_lock lock("/tmp/unique.lock");
        std::cout << "\tunique_file_lock - 1: " << static_cast<bool>(lock) << std::endl;
    }
    {
        os::unique_file_lock lock("/tmp/unique.lock", true);
        std::cout << "\tunique_file_lock - 2: " << static_cast<bool>(lock) << std::endl;
    }
    return 0;
}
