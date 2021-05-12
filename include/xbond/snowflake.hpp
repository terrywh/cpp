#pragma once
#include "vendor.h"

namespace xbond {
class snowflake_node {
    std::uint16_t id_;
public:
    // 默认节点标识（使用 MAC 地址 + PID 哈希生成）
    snowflake_node();
    // 指定节点标识
    snowflake_node(std::uint16_t node)
    : id_(id_) {}
    operator std::uint16_t() const { return id_; }
    // 
    friend std::ostream& operator <<(std::ostream& os, const snowflake_node& node) {
        return os << node.id_;
    }
};
// 
template <class Increment = std::atomic_uint16_t>
class snowflake { 
 public:
    // 
    using incr_type = typename std::decay<Increment>::type;
    // 内存排布（此处按小端形态使得 ID 数值递增）
    struct __attribute__((packed)) value_type {
        std::uint16_t incr_:10;
        std::uint16_t node_:12;
        std::uint64_t time_:42;
    };
    // 指定节点标识及时间起点
    snowflake(snowflake_node node, std::uint64_t epoch = 1500000000000ul)
    : node_(node)
    , epoch_(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - epoch)
    , base_(std::chrono::steady_clock::now()) {}
    // 默认节点标识并指定时间起点
    snowflake(std::uint64_t epoch)
    : snowflake(snowflake_node{}, epoch) {}
    // 内置节点标识及默认时间起点
    snowflake()
    : snowflake(snowflake_node{}, 1500000000000ul) {}
    // 获得下一个 ID 数据
    std::uint64_t next() {
        value_type v {++incr_, node_, time()};
        return *reinterpret_cast<std::uint64_t*>(&v);
    }

 private:
    incr_type     incr_;
    std::uint16_t node_;
    std::uint64_t epoch_;
    std::chrono::steady_clock::time_point base_;
    // 使用 steady_clock 代替 system_clock 计算获取时间
    std::uint64_t time() {
        auto now = std::chrono::steady_clock::now();
        return epoch_ + std::chrono::duration_cast<std::chrono::milliseconds>(now - base_).count();
    }
};

} // namespace xbond
