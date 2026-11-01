#ifndef dns_parser_test_utils_hpp
#define dns_parser_test_utils_hpp

#include <string>
#include <vector>

inline void append_dns_u16(std::vector<unsigned char>& o_packet, unsigned i_value)
{
    o_packet.push_back(static_cast<unsigned char>((i_value >> 8) & 0xff));
    o_packet.push_back(static_cast<unsigned char>(i_value & 0xff));
}

inline void append_dns_name(std::vector<unsigned char>& o_packet, const std::string& i_name)
{
    size_t label_start = 0;
    while (label_start < i_name.size()) {
        const size_t label_end = i_name.find('.', label_start);
        const size_t label_size = (label_end == std::string::npos)
                                      ? i_name.size() - label_start
                                      : label_end - label_start;
        o_packet.push_back(static_cast<unsigned char>(label_size));
        o_packet.insert(o_packet.end(), i_name.begin() + label_start,
                        i_name.begin() + label_start + label_size);
        if (label_end == std::string::npos) { break; }
        label_start = label_end + 1;
    }
    o_packet.push_back(0);
}

inline std::string dns_wire_name(const std::string& i_name)
{
    std::vector<unsigned char> name;
    append_dns_name(name, i_name);
    return std::string(name.begin(), name.end() - 1);
}

inline std::vector<unsigned char> make_dns_question_packet(
    const std::string& i_name = "www.example.com",
    unsigned i_query_type = 1,
    unsigned i_query_class = 1)
{
    std::vector<unsigned char> packet{
        0x12, 0x34, 0x01, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    };
    append_dns_name(packet, i_name);
    append_dns_u16(packet, i_query_type);
    append_dns_u16(packet, i_query_class);
    return packet;
}

inline std::vector<unsigned char> make_dns_a_response_packet()
{
    std::vector<unsigned char> packet = make_dns_question_packet();
    packet[2] = 0x81;
    packet[3] = 0x80;
    packet[6] = 0x00;
    packet[7] = 0x01;

    packet.push_back(0xc0);
    packet.push_back(0x0c);
    append_dns_u16(packet, 1);
    append_dns_u16(packet, 1);
    packet.insert(packet.end(), {0x00, 0x00, 0x00, 0x3c});
    append_dns_u16(packet, 4);
    packet.insert(packet.end(), {1, 2, 3, 4});
    return packet;
}

inline std::vector<unsigned char> make_dns_aaaa_response_packet()
{
    std::vector<unsigned char> packet = make_dns_question_packet("example.com", 28);
    packet[2] = 0x81;
    packet[3] = 0x80;
    packet[6] = 0x00;
    packet[7] = 0x01;

    packet.push_back(0xc0);
    packet.push_back(0x0c);
    append_dns_u16(packet, 28);
    append_dns_u16(packet, 1);
    packet.insert(packet.end(), {0x00, 0x00, 0x00, 0x3c});
    append_dns_u16(packet, 16);
    packet.insert(packet.end(), {
        0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    });
    return packet;
}

inline std::vector<unsigned char> make_dns_cname_response_packet()
{
    std::vector<unsigned char> packet = make_dns_question_packet();
    packet[2] = 0x81;
    packet[3] = 0x80;
    packet[6] = 0x00;
    packet[7] = 0x01;

    packet.push_back(0xc0);
    packet.push_back(0x0c);
    append_dns_u16(packet, 5);
    append_dns_u16(packet, 1);
    packet.insert(packet.end(), {0x00, 0x00, 0x00, 0x3c});
    append_dns_u16(packet, 2);
    packet.push_back(0xc0);
    packet.push_back(0x10);
    return packet;
}

#endif /* dns_parser_test_utils_hpp */
