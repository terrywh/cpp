#include <xbond/strconv/convert.hpp>
using namespace xbond;

#define LOGGER() std::cout

int strconv_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    std::string o = "12345";
    std::string_view s = "54321";
    std::vector<char> v {'9','9','9','9'};
    std::array<char, 3> a {'1','1','1'};
    LOGGER() << "\t" << strconv::str2ul(o) << std::endl;
    LOGGER() << "\t" << strconv::str2ui(s) << std::endl;
    LOGGER() << "\t" << strconv::str2us(v) << std::endl;
    LOGGER() << "\t" << strconv::str2uc(a) << std::endl;
}