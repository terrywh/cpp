#pragma once
#include "../vendor.h"

namespace xbond {
namespace os {

class unique_file_lock {
    int fd_;
    bool locked_;
 public:
    unique_file_lock(std::string_view file, bool block = false);
    ~unique_file_lock();
    operator bool() const { return locked_; }

};

} // namespace os
} // namespace xbond
