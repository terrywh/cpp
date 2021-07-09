#pragma once
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>

namespace xbond {

// 按流方式向容器写入
// 
template <typename C>
struct container_sink {
    typedef typename C::value_type    char_type;
    typedef boost::iostreams::sink_tag category;
    // 设置存储容器
    container_sink(C& c): c_(c) {}
    // 输出方法
    std::streamsize write(const char_type* s, std::streamsize n) {
        c_.insert(c_.end(), s, s+n);
        return n;
    }

private:
    C& c_;
};

} // namespace xbond
