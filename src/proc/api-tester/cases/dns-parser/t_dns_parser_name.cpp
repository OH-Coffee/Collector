#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "dns/dns_parser.hpp"
#include "dns_parser_test_utils.hpp"

namespace {

TEST(DnsParserNameTest, SkipsUncompressedDomainName)
{
    std::vector<unsigned char> packet;
    append_dns_name(packet, "www.example.com");

    EXPECT_EQ(dns_name_skip(packet.data(), 0, static_cast<unsigned>(packet.size())),
              packet.size());
}

TEST(DnsParserNameTest, SkipsCompressedDomainNameAfterValidatingTarget)
{
    std::vector<unsigned char> packet;
    append_dns_name(packet, "www.example.com");
    const unsigned pointer_offset = static_cast<unsigned>(packet.size());
    packet.push_back(0xc0);
    packet.push_back(0x00);

    EXPECT_EQ(dns_name_skip(packet.data(), pointer_offset, static_cast<unsigned>(packet.size())),
              pointer_offset + 2);
}

TEST(DnsParserNameTest, RejectsTruncatedAndInvalidLabels)
{
    const unsigned char truncated_name[] = {3, 'w', 'w'};
    const unsigned char invalid_label[] = {0x40, 0x00};

    EXPECT_EQ(dns_name_skip(truncated_name, 0, sizeof(truncated_name)),
              sizeof(truncated_name) + 1);
    EXPECT_EQ(dns_name_skip(invalid_label, 0, sizeof(invalid_label)),
              sizeof(invalid_label) + 1);
}

TEST(DnsParserNameTest, ExtractsNameThroughCompressionPointer)
{
    std::vector<unsigned char> packet;
    append_dns_name(packet, "www.example.com");
    const unsigned pointer_offset = static_cast<unsigned>(packet.size());
    packet.push_back(0xc0);
    packet.push_back(0x00);

    unsigned char name_buffer[256]{};
    DomainPointer name{ name_buffer, 0 };
    dns_extract_name(packet.data(), pointer_offset, static_cast<unsigned>(packet.size()), &name);

    ASSERT_EQ(name.length, 16U);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(name.name), name.length),
              dns_wire_name("www.example.com"));
}

TEST(DnsParserNameTest, ReadsCompressedNameAsText)
{
    std::vector<unsigned char> packet;
    append_dns_name(packet, "www.example.com");
    unsigned packet_position = static_cast<unsigned>(packet.size());
    packet.push_back(0xc0);
    packet.push_back(0x00);

    char* name = read_rr_name(packet.data(), &packet_position, 0,
                              static_cast<unsigned>(packet.size()));

    ASSERT_NE(name, nullptr);
    EXPECT_STREQ(name, "www.example.com");
    EXPECT_EQ(packet_position, packet.size());
    std::free(name);
}

} // namespace
