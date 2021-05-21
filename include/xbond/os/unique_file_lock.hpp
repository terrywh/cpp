#pragma once
#include "../vendor.h"

namespace xbond {
namespace os {
// 文件（排他）锁
class unique_file_lock {
    int fd_;
    bool locked_;
 public:
    // 尝试进行文件锁，可选的进行阻塞等待（直到获取锁）
    unique_file_lock(std::string_view file, bool block = false);
    ~unique_file_lock();
    operator bool() const { return locked_; }

};

} // namespace os
} // namespace xbond
