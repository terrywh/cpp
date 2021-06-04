#pragma once
#include "../vendor.h"
#include "../detail/to_string_view.hpp"
#include "dynamic_buffer_sink.hpp"
#include "json/detail/parser_handler.hpp"
#include <boost/property_tree/json_parser.hpp>
namespace xbond {
namespace encoding {
namespace json {
// 
inline boost::json::parse_options default_parse_options() {
    static boost::json::parse_options options {
        /* .max_depth = */ 32,
        /* .allow_comments = */true,
        /* .allow_trailing_commas = */ true,
    };
    return options;
}
/**
 * 读取流并解析 JSON 数据
 */
extern void read(std::istream& is, boost::property_tree::ptree& conf,
    const boost::json::parse_options& options = default_parse_options());
/**
 * 读取流并解析 JSON 数据
 */
extern boost::json::value read(std::istream& is,
    const boost::json::parse_options& options = default_parse_options());
// 结合 boost::json::parse 的解析与 boost::property_tree::ptree 容器
// 允许用户使用带注释、结尾逗号的 JSON 数据
template <class S, typename = typename std::enable_if<xbond::detail::to_string_view_invokable<S>::value, S>::type>
static void decode(S json, boost::property_tree::ptree& conf,
    const boost::json::parse_options& options = default_parse_options()) {
    std::string_view sv = xbond::detail::to_string_view(json);
    boost::system::error_code error;
    boost::json::basic_parser<detail::parser_handler> parser(options, conf);
    parser.write_some(false, sv.data(), sv.size(), error);
    if (error) throw boost::system::system_error(error);
    // parser.done();
}
// 结合 boost::json::parse 的解析
// 允许用户使用带注释、结尾逗号的 JSON 数据
template <class S, typename = typename std::enable_if<xbond::detail::to_string_view_invokable<S>::value, S>::type>
static boost::json::value decode(S json,
    const boost::json::parse_options& options = default_parse_options()) {
    std::string_view sv = xbond::detail::to_string_view(json);
    boost::system::error_code error;
    boost::json::parser parser(boost::json::storage_ptr{}, options);
    parser.write(sv.data(), sv.size(), error);
    if (error) throw boost::system::system_error(error);
    return parser.release();
}
/**
 * 序列化 JSON 数据并将其写入流
 */
inline void write(std::ostream& os, const boost::property_tree::ptree& json, bool pretty = true) {
    boost::property_tree::write_json(os, json, pretty);
}
/**
 * 序列化 JSON 数据并将其写入流
 */
void write(std::ostream& os, const boost::json::value& json);
// 序列化
inline std::string encode(const boost::json::value& json);
// 序列化
inline std::string encode(const boost::property_tree::ptree& json, bool pretty = true) {
    std::stringstream ss;
    write(ss, json, pretty);
    return ss.str();
}
/**
 * 序列化 JSON 数据并将其写 DynamicBuffer
 */
template <class DynamicBuffer_v1, class = typename std::enable_if<boost::asio::is_dynamic_buffer_v1<DynamicBuffer_v1>::value>::type>
static void write(DynamicBuffer_v1& v1, const boost::property_tree::ptree& json) {
    // dynamic_buffer_stream_v1<DynamicBuffer_v1> db(v1);
    boost::iostreams::stream<dynamic_buffer_sink_v1<DynamicBuffer_v1>> os(v1);
    write(os, json);
}
/**
 * 序列化 JSON 数据并将其写 DynamicBuffer
 */
template <class DynamicBuffer_v1, class = typename std::enable_if<boost::asio::is_dynamic_buffer_v1<DynamicBuffer_v1>::value>::type>
static void write(DynamicBuffer_v1& v1, const boost::json::value& json) {
    // dynamic_buffer_stream_v1<DynamicBuffer_v1> db(v1);
    boost::iostreams::stream<dynamic_buffer_sink_v1<DynamicBuffer_v1>> os(v1);
    write(os, json);
}

} // namespace json
} // namespace encoding
} // namespace xbond
