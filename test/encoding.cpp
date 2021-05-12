#include <xbond/encoding/base64.hpp>
#include <xbond/encoding/hex.hpp>
#include <xbond/encoding/json.hpp>
#include <xbond/encoding/percent.hpp>
#include <xbond/encoding/utf8.hpp>
#include <iostream>
using namespace xbond;

int encoding_base64_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string o = "中文", r;
    r = encoding::base64::encode(o);
    std::cout << "\t\t" << o  << " -> " << r << std::endl;
    o = encoding::base64::decode(r);
    std::cout << "\t\t" << r  << " -> " << o << std::endl;
    return 0;
}

int encoding_hex_test(int argc, char* argv[]) {
    std::string r = "中文", o;
    
    o = encoding::hex::encode(r);
    std::cout << "\t\t" << r << " -> " << o << std::endl;
    r = encoding::hex::decode(o);
    std::cout << "\t\t" << o << " -> " << r << std::endl;
    return 0;
}

int encoding_json_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string data;
    boost::asio::dynamic_string_buffer buffer(data);
    boost::json::value json {
        {"a", "aaaaa"},
    };
    encoding::json::write(buffer, json);
    std::cout << "\t\t" << data << std::endl;
    return 0;
}

int encoding_percent_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string r = "/1a中文b2-", o;

    o = encoding::percent::encode(r);
    std::cout << "\t\t" << r << " -> " << o << std::endl;
    r = encoding::percent::decode(o);
    std::cout << "\t\t" << o << " -> " << r << std::endl;

    return 0;
}

int encoding_utf8_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string r = "中文", o;
    std::cout << "\t\t" << r << " length: " << encoding::utf8::length(r) << std::endl;
    std::cout << "\t\t" << r << " firstc: " << encoding::utf8::substr(r, 0, 1) << std::endl;
    return 0;
}

int encoding_test(int argc, char* argv[]) {
    std::cout << __func__ << ":\n";
    std::cout << "\t";
    encoding_base64_test(argc, argv);
    std::cout << "\t";
    encoding_percent_test(argc, argv);
    std::cout << "\t";
    encoding_json_test(argc, argv);
    std::cout << "\t";
    encoding_hex_test(argc, argv);
    std::cout << "\t";
    encoding_utf8_test(argc, argv);
}