#pragma once
#include "../../../vendor.h"
#include <boost/json/basic_parser_impl.hpp>
#include <stack>

namespace xbond {
namespace encoding {
namespace json {
namespace detail {

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
    struct context_t {
        boost::property_tree::ptree& node;
        ptree_path path;
    };
    boost::property_tree::ptree& root_;
    std::stack<context_t> context_;
    std::string cache_;

    void put(std::string val) {
        if (context_.top().path.empty()) 
            context_.top().node.push_back(std::make_pair("", boost::property_tree::ptree{val}));
        else
            context_.top().node.put(context_.top().path, val);
    }
    void open() {
        if (context_.empty()) {
            context_.push(context_t{root_});
        }
        else if (context_.top().path.empty()) {
            auto i = context_.top().node.push_back(std::make_pair("", boost::property_tree::ptree{}));
            context_.push(context_t{i->second});
        }
        else {
            auto& child = context_.top().node.put_child(context_.top().path, {});
            context_.push(context_t{child});
        }
    }
    void done() { // 数据项结束，回到父级别键
        if (!context_.empty() && !context_.top().path.empty()) context_.top().path.pop_back();
    }

public:
    parser_handler(boost::property_tree::ptree& ptree)
    : root_(ptree) {
        
    }
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
        open();
        return true;
    }
    bool on_array_end(std::size_t n, boost::system::error_code& ec) {
        context_.pop();
        done();
        return true;
    }
    bool on_object_begin(boost::system::error_code& ec) { 
        open();
        return true;
    }
    bool on_object_end(std::size_t n, boost::system::error_code& ec) {
        context_.pop();
        done();
        return true;
    }
    bool on_string_part(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
        cache_.append(s.data(), s.size());
        return true;
    }
    bool on_string(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
        cache_.append(s.data(), s.size());
        put(cache_);
        cache_.clear(); // 新数据项或键直接开始追加
        if (!context_.top().path.empty()) context_.top().path.pop_back(); 
        return true;
    }
    bool on_key_part(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
        cache_.append(s.data(), s.size());
        return true;
    }
    bool on_key(boost::string_view s, std::size_t n, boost::system::error_code& ec) {
        cache_.append(s.data(), s.size());
        context_.top().path.push_back(std::move(cache_));
        cache_.clear();
        return true;
    }
    bool on_number_part(boost::string_view s, boost::system::error_code& ec ) { return true; }
    bool on_int64(int64_t i, boost::string_view s, boost::system::error_code& ec) {
        put({s.data(), s.size()});
        done();
        return true;
    }
    bool on_uint64(uint64_t u, boost::string_view s, boost::system::error_code& ec ) {
        put({s.data(), s.size()});
        done();
        return true;
    }
    bool on_double(double d, boost::string_view s, boost::system::error_code& ec) {
        put({s.data(), s.size()});
        done();
        return true;
    }
    bool on_bool(bool b, boost::system::error_code& ec) {
        put(b ? "true" : "false");
        done();
        return true;
    }
    bool on_null(boost::system::error_code& ec) {
        put("null");
        done();
        return true;
    }
    bool on_comment_part(boost::string_view s, boost::system::error_code& ec) { return true; }
    bool on_comment(boost::string_view s, boost::system::error_code& ec) { return true; }
};

}
}
}
}