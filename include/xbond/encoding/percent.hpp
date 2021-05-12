#pragma once
#include <iterator>
#include <boost/algorithm/hex.hpp>

namespace xbond {
namespace encoding {

class percent {
    // 表码表
    __attribute__(( __aligned__ (16)))
    static constexpr char HEX_CONVERT_TABLE[] = "0123456789ABCDEF";

 public:
    // 
    template <class InputIterator, class OutputIterator>
    static OutputIterator encode(InputIterator begin, InputIterator end, OutputIterator to) {
        auto j = to;
        for(auto i = begin; i!=end; ++i) {
            unsigned char c = *i;
            if(std::isalnum(c) || c=='-' || c== '.' || c=='_')
                *j = c;
            else {
                *j = '%';
                ++j;
                *j = HEX_CONVERT_TABLE[c >> 4];
                ++j;
                *j = HEX_CONVERT_TABLE[c & 0x0f];
            }
            ++j;
        }
        return j;
    }
    // 
    template <class StringView>
    static std::string encode(const StringView& sv) {
        std::string str;
        str.reserve(sv.size() * 2);
        encode(sv.data(), sv.data() + sv.size(),
            std::back_insert_iterator<std::string>(str));
        return str;
    }
    // 
    static unsigned char htoi(unsigned char s1, unsigned char s2) {
        unsigned char value;
        if (std::isupper(s1)) s1 = std::tolower(s1);
        value = (s1 >= '0' && s1 <= '9' ? s1 - '0' : s1 - 'a' + 10) << 4;
        if (std::isupper(s2)) s2 = std::tolower(s2);
        value |= s2 >= '0' && s2 <= '9' ? s2 - '0' : s2 - 'a' + 10;
        return value;
    }
    // 
    template <class InputIterator, class OutputIterator>
    static OutputIterator decode(InputIterator begin, InputIterator end, OutputIterator to) {
        auto j = to;
        for(auto i=begin; i!=end; ++i) {
            if(*i == '+') *j = ' ';
            else if(*i == '%' && i+1 != end && i+2 != end
                && std::isxdigit(static_cast<int>(*(i+1)))
                && std::isxdigit(static_cast<int>(*(i+2)))) {
                
                *j = static_cast<char>(htoi(*(i+1), *(i+2)));
                ++i;
                ++i;
            }
            else *j = *i;
            ++j;
        }
        return j;
    }
    // 
    template <class StringView>
    static std::string decode(const StringView& sv) {
        std::string str;
        str.reserve(sv.size());
        decode(sv.data(), sv.data() + sv.size(), std::back_insert_iterator<std::string>(str));
        return str;
    }
};

} // namespace encoding
} // namespace xbond
