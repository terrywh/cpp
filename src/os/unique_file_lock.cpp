#include <xbond/os/unique_file_lock.hpp>
#include <boost/filesystem/path.hpp>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>


namespace xbond {
namespace os {

unique_file_lock::unique_file_lock(std::string_view file, bool block)
: locked_(false) {
    boost::filesystem::path path {file.data(), file.data() + file.size()};
    fd_ = open(path.c_str(), O_CREAT | O_RDWR, 0666);
    if (block && flock(fd_, LOCK_EX) == 0 || // 阻塞等待锁
        flock(fd_, LOCK_EX | LOCK_NB) == 0) // 非阻塞锁
        locked_ = true;
}
unique_file_lock::~unique_file_lock() {
    flock(fd_, LOCK_UN);
    close(fd_);
}

} // namespace os
} // namespace xbond
