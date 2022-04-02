#pragma once
#include "device.hpp"
#include <sstream>

namespace xbond {
namespace net {
// 物理地址（MAC）
class hardware_address {
public:
    using value_type = std::array<std::uint8_t, 6>;
    // 尝试获取当前机器的物理地址
    hardware_address() {
        foreach_device([this] (const device_info& device) -> bool {
            if (device.name[0] == 'e') {
                hwaddr_ = device.hw;
                return false;
            }
            return true;
        });
    }
    // 以指定数据构建一个物理地址实例
    hardware_address(const value_type& bytes)
    : hwaddr_(bytes) {}
    // 物理地址数据
    value_type& bytes() { return hwaddr_; }
    // 构建并返回物理地址的文本
    std::string str() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

 private:
    value_type hwaddr_;
    friend std::ostream& operator<<(std::ostream& os, const hardware_address& ha);
};

} // namespace net
} // namespace xbond

