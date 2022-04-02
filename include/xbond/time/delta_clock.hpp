#pragma once
#include "date.hpp"
#include <atomic>

namespace xbond {
namespace time {
    // 偏移校准时钟
    template <typename DIFF_TYPE = std::atomic_int64_t>
    class basic_delta_clock {
        DIFF_TYPE diff_; // 差距精度：毫秒

     public:
        static basic_delta_clock& get() {
            static basic_delta_clock clock;
            return clock;
        }
        // 构建，可选远端参照时间
        explicit basic_delta_clock(std::chrono::milliseconds remote =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())) {
            swap(remote);
        }
        // 系统时钟 
        operator std::chrono::system_clock::time_point() const { 
            return std::chrono::system_clock::time_point(
                std::chrono::milliseconds( static_cast<std::int64_t>(*this) ));
        }
        // 时间戳：毫秒
        operator std::int64_t() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() + diff_;
        }
        // 调整参照时间
        void swap(std::chrono::milliseconds r) {
            std::int64_t local = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count(),
                remote = std::chrono::duration_cast<std::chrono::milliseconds>( r ).count();

            diff_ = remote - local;
        }
    };

    using delta_clock = basic_delta_clock<std::atomic_int64_t>;

} // namespace time
} // namespace xbond
