#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "dns/dns_parser.hpp"
#include "dns_parser_test_utils.hpp"

namespace {

TEST(DnsParserProtocolTest, ParsesDnsQuestionHeaderAndQueryName)
{
    const std::vector<unsigned char> packet = make_dns_question_packet();
    DNS_Incoming dns{};

    proto_dns_parse(&dns, packet.data(), 0, static_cast<unsigned>(packet.size()));

    EXPECT_TRUE(dns.is_valid);
    EXPECT_FALSE(dns.is_formerr);
    EXPECT_EQ(dns.id, 0x1234U);
    EXPECT_EQ(dns.qdcount, 1U);
    EXPECT_EQ(dns.query_type, 1U);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(dns.query_name.name),
                          dns.query_name.length),
                          dns_wire_name("www.example.com"));
}

TEST(DnsParserProtocolTest, ParsesQueryResultAndIpv4Answer)
{
    const std::vector<unsigned char> packet = make_dns_a_response_packet();
    DnsParseResult result{};

    ASSERT_TRUE(parse_dns_protocol(packet.data(), static_cast<unsigned>(packet.size()), &result));
    EXPECT_EQ(result.QueryName, "www.example.com");
    EXPECT_EQ(result.QueryType, 1);
    EXPECT_EQ(result.QueryResult, 0);
    EXPECT_EQ(result.QueryOptions, 0);
    EXPECT_EQ(result.AnswerName, "type: 1 1.2.3.4;");
}

TEST(DnsParserProtocolTest, ParsesIpv6AndCnameAnswers)
{
    const std::vector<unsigned char> aaaa_packet = make_dns_aaaa_response_packet();
    DnsParseResult aaaa_result{};

    ASSERT_TRUE(parse_dns_protocol(aaaa_packet.data(),
                                   static_cast<unsigned>(aaaa_packet.size()),
                                   &aaaa_result));
    EXPECT_EQ(aaaa_result.QueryName, "example.com");
    EXPECT_EQ(aaaa_result.QueryType, 28);
    EXPECT_EQ(aaaa_result.AnswerName, "type: 28 2001:db8::1;");

    const std::vector<unsigned char> cname_packet = make_dns_cname_response_packet();
    DnsParseResult cname_result{};
    ASSERT_TRUE(parse_dns_protocol(cname_packet.data(),
                                   static_cast<unsigned>(cname_packet.size()),
                                   &cname_result));
    EXPECT_EQ(cname_result.AnswerName, "type: 5 example.com;");
}

TEST(DnsParserProtocolTest, MapsDnsResponseCodeToQueryResult)
{
    std::vector<unsigned char> packet = make_dns_question_packet();
    packet[2] = 0x81;
    packet[3] = 0x83;
    DnsParseResult result{};

    ASSERT_TRUE(parse_dns_protocol(packet.data(), static_cast<unsigned>(packet.size()), &result));
    EXPECT_EQ(result.QueryResult, 9003);
}

TEST(DnsParserProtocolTest, RejectsNullShortAndOversizedPackets)
{
    DnsParseResult result{};
    const unsigned char short_packet[] = {0x12, 0x34, 0x01};
    const std::vector<unsigned char> packet = make_dns_question_packet();
    const std::vector<unsigned char> oversized_packet(0x1000);

    EXPECT_FALSE(parse_dns_protocol(nullptr, 12, &result));
    EXPECT_FALSE(parse_dns_protocol(short_packet, sizeof(short_packet), &result));
    EXPECT_FALSE(parse_dns_protocol(oversized_packet.data(),
                                    static_cast<unsigned>(oversized_packet.size()),
                                    &result));
}

TEST(DnsParserProtocolTest, RejectsInvalidQuestionAndTruncatedAnswer)
{
    std::vector<unsigned char> invalid_question = make_dns_question_packet("example.com", 1, 2);
    DnsParseResult result{};
    EXPECT_FALSE(parse_dns_protocol(invalid_question.data(),
                                    static_cast<unsigned>(invalid_question.size()),
                                    &result));

    std::vector<unsigned char> truncated_answer = make_dns_a_response_packet();
    truncated_answer.pop_back();
    EXPECT_FALSE(parse_dns_protocol(truncated_answer.data(),
                                    static_cast<unsigned>(truncated_answer.size()),
                                    &result));
}

} // namespace
