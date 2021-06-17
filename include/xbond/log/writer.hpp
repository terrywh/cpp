#pragma once
#include "../vendor.h"
#include "basic_writer.hpp"

namespace xbond {
namespace log {


/**
 * 标准输出器
 * @note 非线程安全
 */
class cout_writer {
 public:
    void operator()(std::shared_ptr<basic_record> r) {
        std::string_view s = *r;
        std::cout.write(s.data(), s.size());
        std::cout.flush();
    }
};
/**
 * 文件输出器
 * @note 非线程安全
 */
class file_writer {
    boost::filesystem::path path_;
    std::shared_ptr<std::ofstream> file_;

 public:
    file_writer(const char* file, bool create_directory = false);
    void operator()(std::shared_ptr<basic_record> r);
    void rotate();
};

} // namespace log
} // namespace xbond
