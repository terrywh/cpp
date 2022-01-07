#include <xbond/encoding/json.hpp>
#include <boost/json/stream_parser.hpp>
#include <boost/json/serializer.hpp>

namespace xbond {
namespace encoding {
namespace json {
/**
    * 读取流并解析 JSON 数据
    */
void read(std::istream& is, boost::property_tree::ptree& conf, const boost::json::parse_options& options) {
    assert(is.good());
    boost::system::error_code   error;
    boost::json::basic_parser<detail::parser_handler> parser(options, conf);

    std::array<char, 4096> buffer;
    do {
        is.read(buffer.data(), sizeof(buffer));
        parser.write_some(!is.eof(), buffer.data(), is.gcount(), error);
    } while(!is.eof());
    if (error) throw boost::system::system_error(error);
}
/**
    * 读取流并解析 JSON 数据
    */
boost::json::value read(std::istream& is, const boost::json::parse_options& options) {
    boost::json::stream_parser parser(boost::json::storage_ptr{}, options);
    boost::system::error_code error;

    std::array<char, 4096> buffer;
    do {
        is.read(buffer.data(), sizeof(buffer));
        parser.write_some(buffer.data(), is.gcount(), error);
    } while(!is.eof());
    if (error) throw boost::system::system_error(error);
    return parser.release();
}
/**
    * 序列化 JSON 数据并将其写入流
    */
void write(std::ostream& os, const boost::json::value& json) {
    boost::json::serializer sr;
    sr.reset(&json);
    std::array<char, 4096> buffer;
    while (!sr.done()) {
        os << sr.read(buffer.data(), buffer.size());
    }
}

} // namespace json
} // namespace encoding
} // namespace xbond
