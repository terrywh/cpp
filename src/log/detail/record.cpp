#include <xbond/log/detail/record.hpp>
#include <xbond/time/date.hpp>
#include <xbond/time/delta_clock.hpp>
#include <boost/pool/pool.hpp>

namespace xbond {
namespace log {
namespace detail {

static boost::pool<> record_pool (sizeof(record));

void* record::operator new(std::size_t size) {
    return record_pool.malloc();
}

void record::operator delete(void *p) {
    record_pool.free(p);
}

} // namespace detail
} // namespace log
} // namespace xbond
