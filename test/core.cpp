#include <xbond/snowflake.hpp>
#include <xbond/reference_pointer.hpp>
#include <boost/json.hpp>
#include <iostream>
using namespace xbond;

extern int core_coroutine_test(int argc, char* argv[]);

int core_test(int argc, char* argv[]) {
    std::cout << __func__ << ":\n";
    snowflake sf;
    for (int i=0; i<10; ++i)
        std::cout << "\tsnowflake = " << sf.next() << "\n";

    auto r = std::make_shared<std::function<void (boost::system::error_code error)>>([] (boost::system::error_code error) {
        std::cout << "\treference_ptr (error_code)\n";
    });
    reference_pointer(std::move(r))({});
    
    core_coroutine_test(argc, argv);
    return 0;
}