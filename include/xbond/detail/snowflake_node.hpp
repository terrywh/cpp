#pragma once
#include <ostream>
#include <cstdint>

namespace xbond {
namespace detail {
class snowflake_node {
    std::uint16_t id_;

 public:
    // 默认节点标识（使用 MAC 地址 + PID 哈希生成）
    snowflake_node();
    // 指定节点标识
    snowflake_node(std::uint16_t node)
    : id_(node) {}
    // 
    operator std::uint16_t() const { return id_; }
    // 
    friend std::ostream& operator <<(std::ostream& os, const snowflake_node& node) {
        return os << node.id_;
    }
};

} // namespace detail
} // namespace xbond
