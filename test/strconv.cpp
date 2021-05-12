#include <xbond/strconv/convert.hpp>
using namespace xbond;

int strconv_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string o = "12345";
    std::cout << "\t" << strconv::str2ul(o) << std::endl;
    std::cout << "\t" << strconv::str2ui(o) << std::endl;
    std::cout << "\t" << strconv::str2us(o) << std::endl;
}