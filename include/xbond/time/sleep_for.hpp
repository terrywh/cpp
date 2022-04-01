#pragma once
#include "timer.hpp"

namespace xbond {
namespace time {

// 暂停当前协程（一段时间）
void sleep_for(std::chrono::steady_clock::duration duration, xbond::coroutine_handler& ch);
// 暂停当前协程（一段时间）
void sleep_for(std::chrono::steady_clock::duration duration);
} // namespace time
} // namespace xbond
