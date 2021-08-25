#include <xbond/strconv/parse_string.hpp>
using namespace xbond;

#define LOGGER() std::cout

int strconv_test(int argc, char* argv[]) {
    LOGGER() << __func__ << "\n";
    std::string o = "12345";
    std::string_view s = "54321";
    std::vector<char> v {'9','9','9','9'};
    std::array<char, 3> a {'1','1','1'};
    
    LOGGER() << "\t" << static_cast<std::uint64_t>(strconv::parse_string(o)) << std::endl;
    LOGGER() << "\t" << static_cast<std::uint64_t>(strconv::parse_string(s)) << std::endl;
    LOGGER() << "\t" << static_cast<std::uint64_t>(strconv::parse_string(v)) << std::endl;
    LOGGER() << "\t" << static_cast<std::uint64_t>(strconv::parse_string(a)) << std::endl;
    LOGGER() << "\t" << static_cast<std::uint64_t>(strconv::parse_string("38348847570747393")) << std::endl;
    LOGGER() << "\t" << static_cast<std::uint16_t>(strconv::parse_string("90999")) << std::endl;
}