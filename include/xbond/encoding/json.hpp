#pragma once
#include "../vendor.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>

namespace xbond {
namespace encoding {

class json {

    struct ptree_path {
        bool empty() const { return path_.empty(); }
        operator boost::property_tree::path() const { return path_; }
        void push_back(std::string key) {
            if (!path_.empty()) path_.push_back('.'); // 根下元素无需额外的 . 分隔符
            path_.append(key);
        }
        void pop_back() {
            if (path_.empty()) return; // 已上升的根路径
            std::size_t p = path_.find_last_of('.');
            if (p == std::string::npos) path_.clear();// 最后一节，直接清理即可
            else path_.resize(p);
        }
    private:
        std::string path_;
        friend std::ostream& operator <<(std::ostream& os, const ptree_path& path) {
            return os << path.path_;
        }
    };

    struct parser_handler {
    private:
        boost::property_tree::ptree& ptree_;
        ptree_path ppath_;
        std::string tmp_;

    public:
        parser_handler(boost::property_tree::ptree& ptree)
        : ptree_(ptree) {}
        /// The maximum number of elements allowed in an array
        static constexpr std::size_t max_array_size = -1;
        /// The maximum number of elements allowed in an object
        static constexpr std::size_t max_object_size = -1;
        /// The maximum number of characters allowed in a string
        static constexpr std::size_t max_string_size = -1;
        /// The maximum number of characters allowed in a key
        static constexpr std::size_t max_key_size = -1;
        bool on_document_begin(boost::system::error_code& ec) { return true; }
        bool on_document_end(boost::system::error_code& ec) { return true; }
        bool on_array_begin(boost::system::error_code& ec) {
            ppath_.push_back("");
            return true;
        }
        bool on_array_end(std::size_t n, boost::system::error_code& ec) {
            ppath_.pop_back();
            return true;
        }
        bool on_object_begin(boost::system::error_code& ec) { return true; }
        bool on_object_end(std::size_t n, boost::system::error_code& ec) {
            if (!ppath_.empty()) ppath_.pop_back();
            return true;
        }
        bool on_string_part(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
            tmp_.append(s.data(), s.size());
            return true;
        }
        bool on_string(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
            tmp_.append(s.data(), s.size());
            ptree_.put(ppath_, tmp_);
            tmp_.clear(); // 新数据项或键直接开始追加
            ppath_.pop_back(); // 数据项结束，回到父级别键
            return true;
        }
        bool on_key_part(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
            tmp_.append(s.data(), s.size());
            return true;
        }
        bool on_key(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
            tmp_.append(s.data(), s.size());
            ppath_.push_back(std::move(tmp_));
            tmp_.clear();
            return true;
        }
        bool on_number_part(boost::string_view s, boost::system::error_code& ec ) { return true; }
        bool on_int64(int64_t i, boost::string_view s, boost::system::error_code& ec) {
            ptree_.put(ppath_, i);
            ppath_.pop_back(); // 数据项结束，回到父级别键
            return true;
        }
        bool on_uint64(uint64_t u, boost::string_view s, boost::system::error_code& ec ) {
            ptree_.put(ppath_, u);
            ppath_.pop_back(); // 数据项结束，回到父级别键
            return true;
        }
        bool on_double(double d, boost::string_view s, boost::system::error_code& ec) {
            ptree_.put(ppath_, d);
            ppath_.pop_back(); // 数据项结束，回到父级别键
            return true;
        }
        bool on_bool(bool b, boost::system::error_code& ec) {
            ptree_.put(ppath_, b);
            ppath_.pop_back(); // 数据项结束，回到父级别键
            return true;
        }
        bool on_null(boost::system::error_code& ec) {
            ptree_.put(ppath_, "null");
            ppath_.pop_back(); // 数据项结束，回到父级别键
            return true;
        }
        bool on_comment_part(boost::string_view s, boost::system::error_code& ec) { return true; }
        bool on_comment(boost::string_view s, boost::system::error_code& ec) { return true; }
    };
    using parser_type = boost::json::basic_parser<parser_handler>;

    template <class DynamicBuffer_v1>
    class dynamic_buffer_stream_v1: public boost::iostreams::sink {
        DynamicBuffer_v1& buffer_;
     public:
        dynamic_buffer_stream_v1(DynamicBuffer_v1& v1)
        : buffer_(v1) {}
        std::streamsize write(const char* data, std::streamsize size) {
            auto buffers = buffer_.prepare(size);
            const char* offset = data;
            std::size_t remain = size;
            for (auto i=boost::asio::buffer_sequence_begin(buffers); 
                remain > 0 && i!=boost::asio::buffer_sequence_end(buffers); ++i) {
                boost::asio::mutable_buffer mb = *i;
                std::size_t chunk_size = mb.size() < remain ? mb.size() : remain;
                std::memcpy(mb.data(), offset, chunk_size);
                offset += chunk_size;
                remain -= chunk_size;
            }
            return size - remain;
        }
    };

 public:
    // 
    static boost::json::parse_options default_parse_options() {
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
    static void read(std::istream& is, boost::property_tree::ptree& conf,
        const boost::json::parse_options& options = default_parse_options()) {
        assert(is.good());
        boost::system::error_code   error;
        parser_type parser(options, conf);

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
    static boost::json::value read(std::istream& is,
        const boost::json::parse_options& options = default_parse_options()) {
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
    // 结合 boost::json::parse 的解析与 boost::property_tree::ptree 容器
    // 允许用户使用带注释、结尾逗号的 JSON 数据
    template <class StringView, typename = typename std::enable_if<std::is_convertible<StringView, std::string_view>::value, StringView>::type>
    static void decode(StringView json, boost::property_tree::ptree& conf,
        const boost::json::parse_options& options = default_parse_options()) {
        std::string_view sv = json;
        boost::system::error_code error;
        parser_type parser(options, conf);
        parser.write_some(false, sv.data(), sv.size(), error);
        if (error) throw boost::system::system_error(error);
        // parser.done();
    }
    template <class StringView, typename = typename std::enable_if<std::is_convertible<StringView, std::string_view>::value, StringView>::type>
    static boost::json::value decode(StringView json,
        const boost::json::parse_options& options = default_parse_options()) {
        std::string_view sv = json;
        boost::system::error_code error;
        boost::json::parser parser(boost::json::storage_ptr{}, options);
        parser.write(sv.data(), sv.size(), error);
        if (error) throw boost::system::system_error(error);
        return parser.release();
    }
    /**
     * 序列化 JSON 数据并将其写入流
     */
    static void write(std::ostream& os, const boost::property_tree::ptree& json, bool pretty = true) {
        boost::property_tree::write_json(os, json, pretty);
    }
    /**
     * 序列化 JSON 数据并将其写入流
     */
    static void write(std::ostream& os, const boost::json::value& json) {
        boost::json::serializer sr;
        sr.reset(&json);
        std::array<char, 4096> buffer;
        while (!sr.done()) {
            os << sr.read(buffer.data(), buffer.size());
        }
    }
    // 序列化
    static std::string encode(const boost::json::value& json) {
        return boost::json::serialize(json);
    }
    //
    static std::string encode(const boost::property_tree::ptree& json, bool pretty = true) {
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
        boost::iostreams::stream<dynamic_buffer_stream_v1<DynamicBuffer_v1>> os(v1);
        write(os, json);
    }
    /**
     * 序列化 JSON 数据并将其写 DynamicBuffer
     */
    template <class DynamicBuffer_v1, class = typename std::enable_if<boost::asio::is_dynamic_buffer_v1<DynamicBuffer_v1>::value>::type>
    static void write(DynamicBuffer_v1& v1, const boost::json::value& json) {
        // dynamic_buffer_stream_v1<DynamicBuffer_v1> db(v1);
        boost::iostreams::stream<dynamic_buffer_stream_v1<DynamicBuffer_v1>> os(v1);
        write(os, json);
    }
};

} // namespace encoding
} // namespace json
