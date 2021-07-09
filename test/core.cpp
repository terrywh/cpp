#include <xbond/snowflake.hpp>
#include <iostream>
using namespace xbond;

extern int core_coroutine_test(int argc, char* argv[]);

int core_test(int argc, char* argv[]) {
    std::cout << __func__ << ":\n\t";

    boost::json::value a = boost::json::parse(R"({"a":1})");
    std::cout << a << std::endl;
    auto x = boost::json::value_to<std::uint64_t>(a.get_object().at("a"));
    std::cout << x << std::endl;

    core_coroutine_test(argc, argv);
}