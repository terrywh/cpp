#pragma once
#include "detail/write_json.hpp"
#include <boost/asio/compose.hpp>

namespace xbond {
namespace net {
/**
 * 异步向流中写入 JSON 数据（一般用于较大量 JSON 数据）
 * @param data 待写入数据项，由调用者持有，须在回调前有效
 * @remark
 *  此操作为 组合操作 Composed Operation 调用者须保证在回调执行前，不进行其他形式的写操作 async_write_some() 调用；
 */
template <class AsyncWriteStream, class CompletionToken, std::size_t BufferSize = 16 * 1024>
void async_write(AsyncWriteStream& stream, boost::json::value& data, CompletionToken&& handler) {
    return boost::asio::async_compose<CompletionToken, void(boost::system::error_code)>(
        detail::write_json<AsyncWriteStream, BufferSize>(stream, data), handler, stream);
}

} // namespace net
} // namespace xbond
