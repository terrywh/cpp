#include <xbond/snowflake.hpp>
#include <iostream>
using namespace xbond;

extern int core_coroutine_test(int argc, char* argv[]);

int core_test(int argc, char* argv[]) {
    std::cout << __func__ << ":\n\t";
    core_coroutine_test(argc, argv);
}