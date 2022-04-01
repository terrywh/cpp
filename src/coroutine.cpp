#include <xbond/coroutine.hpp>

namespace xbond {
thread_local std::weak_ptr<coroutine> coroutine::current_;
} // namespace xbond
