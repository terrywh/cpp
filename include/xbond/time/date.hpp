#pragma once
#include <date/date.h>
#include <date/tz.h>

namespace xbond {
namespace time {

using namespace date;

std::string iso(std::chrono::system_clock::time_point time_point, std::chrono::hours zone_offset = std::chrono::hours(8));

} // namespace time
} // namespace xbond;
