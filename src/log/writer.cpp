#include "boost/asio/executor_work_guard.hpp"
#include "boost/asio/io_context.hpp"
#include <xbond/log/writer.hpp>

namespace xbond {
namespace log {

file_writer::file_writer(const char* file, bool create_directory)
: path_(file) {
    if (create_directory) {
        boost::filesystem::path path(file);
        boost::filesystem::create_directories(path.parent_path());
    }
    rotate();
}

void file_writer::operator()(std::shared_ptr<basic_record> r) {
    std::string_view s = *r;
    file_->write(s.data(), s.size());
    file_->flush(); // 按写入条目输出
}

void file_writer::rotate() {
    file_ = std::make_shared<std::ofstream>(path_.c_str(), std::ios_base::ate | std::ios_base::out | std::ios_base::app);
    if (!file_->is_open()) 
        throw std::runtime_error("cannot create/open file for append");
}


} // namespace log
} // namespace xbond
