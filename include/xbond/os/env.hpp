#pragma once
#include <boost/filesystem/path.hpp>

namespace xbond {
namespace os {
// 获取当前（正在运行的）可执行文件路径
extern const boost::filesystem::path& executable();

} // namespace os
} // namespace xbond
