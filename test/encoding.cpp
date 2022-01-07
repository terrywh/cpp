#include <xbond/encoding/base64.hpp>
#include <xbond/encoding/hex.hpp>
#include <xbond/encoding/json.hpp>
#include <xbond/encoding/percent.hpp>
#include <xbond/encoding/utf8.hpp>
#include <libbase64.h>
#include <iostream>
using namespace xbond;

static std::string load_from_file(const char* file) {
    std::ifstream fs("libxbond.a");
    fs.seekg(0, std::ifstream::end);
    std::size_t size = fs.tellg();
    std::string data;
    data.resize(size);
    fs.read(data.data(), size);
    return data;
}

static std::string large_data = load_from_file("libxbond.a");

int encoding_base64_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
    std::string o, r;
   
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i=0;i<100;++i) {
        r = encoding::base64::encode(large_data);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "\t\tencode -> " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms\n";
    
    std::size_t outl;
    begin = std::chrono::high_resolution_clock::now();
    for (int i=0;i<100;++i) {
        o = encoding::base64::decode(r);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "\t\tdecode -> " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms\n";
    o = "1中2国3";
    r = encoding::base64::encode(o);
    std::cout << "\t\t" << o  << " -> " << r << std::endl;
    r = encoding::base64::encode("aaaaaaaaaaaaaaaaa1222222222222222222222222222222222222");
    std::cout << "\t\t" << o  << " -> " << r << std::endl;
    o = encoding::base64::decode(r);
    std::cout << "\t\t" << r  << " -> " << o << std::endl;
    return 0;
}

int encoding_hex_test(int argc, char* argv[]) {
    std::cout << __func__ << "\n";
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

    boost::property_tree::ptree ctr1;
    std::string raw1 = R"({"msg_s2c_user_status_change_notify_req_body":{"enum_event_type":0,"msg_user_status_list":[{"msg_app_user":{"msg_user_key":{"enum_access_type":2,"msg_phone":{"str_phone_number":"testor1","enum_device_type":1,"uint32_app_id":200000003}},"msg_user_extent_info":{"bytes_nick_name":"ZEdWemRHOXlYM05wYlY5a1pYWT0=","str_contact_phone_number_list":["testor1"],"uint64_service_platform_tiny_id":144115233275364998,"enum_user_role":1,"bytes_avatar_url":"aHR0cHM6Ly93b3JrLm1lZGlhbGFiLnFxLmNvbS8/YXV0aF90eXBlPW9haWNvbiZhcHBfdWlkPXRlc3RvcjEmYXBwX2lkPTIwMDAwMDAwMyZ0aW1lPTE2MjE5NDM0NzImc2l6ZT0xNTAmdG9rZW49NjU3Nzg5MmJhZTA5M2ZjY2ZhMmZkNGM5YzQ1M2QyOWU3NzY1MzNmZmE0MDUzZjMwY2VjMzdiYTYyYTM4ZmJlYg==","str_pstn_call_id":"","enum_user_sub_type":2}},"bool_is_creator":false,"bool_is_host":false,"msg_media_status":{"msg_audio":{"enum_stream_switch_status_up":0,"bool_has_mic":false,"uint32_action_reason":0},"msg_video":{"enum_stream_switch_status_up":0,"bool_has_camera":false,"uint32_action_reason":0},"msg_share_screen":{"enum_stream_switch_status_up":0,"uint32_action_reason":0},"msg_share_audio":{"enum_stream_switch_status_up":0,"uint32_action_reason":0}},"uint64_member_state_last_update_time":1621943472806,"uint32_role_type":3,"uint32_member_concurrent_seq":166,"uint32_avatar_type":0,"uint32_user_action":0,"uint32_user_district":0,"uint32_local_record_status":0,"uint32_allow_directed_in_meeting":0,"bool_is_support_local_record":false,"uint64_scenes_bits":0,"uint32_allow_speak":0}],"bool_is_push_end":true}})";
    encoding::json::decode(raw1, ctr1);

    auto front = ctr1.get_child("msg_s2c_user_status_change_notify_req_body.msg_user_status_list").front().second;
    std::cout << "\t\t" << front.get_child("msg_app_user.msg_user_extent_info.str_contact_phone_number_list").front().second.get_value<std::string>() << std::endl;

    boost::property_tree::ptree ctr2;
    std::string raw2 = R"([{"A":"a"}, "a"])";
    encoding::json::decode(raw2, ctr2);
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
    std::string r = "中文人", o;
    std::cout << "\t\t" << r << " length: " << encoding::utf8::length(r) << std::endl;
    std::cout << "\t\t" << r << " firstc: " << encoding::utf8::substr(r, 0, 1) << std::endl;
    std::cout << "\t\t" << r << " second: " << encoding::utf8::substr(r, 1, 1) << std::endl;
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
    return 0;
}