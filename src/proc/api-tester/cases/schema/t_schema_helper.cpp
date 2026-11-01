#include <cstring>
#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "jsoncpp/json/json.h"
#include "schema/schema_helper.hpp"

namespace {

TEST(SchemaHelperTest, InitializesAndValidatesSchema)
{
    SCHEMA schema;
    ASSERT_EQ(schema_init(&schema, sizeof(schema)), RC_OK);

    EXPECT_STREQ(schema.Head.magic, SCHEMA_HEAD_MAGIC);
    EXPECT_EQ(schema.Head.version, static_cast<uint64_t>(SCHEMA_HEAD_VERSION));
    EXPECT_EQ(schema.Head.type, SCHEMA_UNKNOWN);
    EXPECT_EQ(schema.Head.event_type, EVENT_TYPE_UNKNOWN);
    EXPECT_EQ(schema.Head.event_class, EVENT_CLASS_UNKNOWN);
    EXPECT_EQ(schema.Head.data_len, uint64_t{0});
    EXPECT_EQ(schema_check(&schema), RC_OK);

    EXPECT_EQ(schema_init(&schema, sizeof(schema) - 1), RC_ARG_ERR);
    EXPECT_EQ(schema_init(nullptr, sizeof(schema)), RC_ARG_ERR);
    EXPECT_EQ(schema_check(nullptr), RC_ARG_ERR);

    schema.Head.version++;
    EXPECT_EQ(schema_check(&schema), RC_FAIL);
}

TEST(SchemaHelperTest, SerializesSchemaHeaderAndProcessBody)
{
    SCHEMA schema{};
    ASSERT_EQ(schema_init(&schema, sizeof(schema)), RC_OK);
    schema.Head.sequence = 5;
    schema.Head.type = SCHEMA_UNKNOWN;
    schema.Head.rule_id = 21;
    schema.Head.pid = 100;
    schema.Head.ppid = 99;
    schema.Head.event_type = EVENT_TYPE_EXEC;
    schema.Head.event_class = EVENT_CLASS_NOTIFY;
    schema.Head.data_len = sizeof(EVENT_PROC_EXEC);
    schema.Body.exec.pid = 100;
    schema.Body.exec.ppid = 99;
    schema.Body.exec.uid = 501;
    std::strcpy(reinterpret_cast<char*>(schema.Body.exec.name), "collector");
    std::strcpy(reinterpret_cast<char*>(schema.Body.exec.path), "/usr/bin/collector");
    std::strcpy(reinterpret_cast<char*>(schema.Body.exec.args), "--run");

    Json::Value json;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(schema2string(schema), json));
    EXPECT_EQ(json["head"]["magic"].asString(), SCHEMA_HEAD_MAGIC);
    EXPECT_EQ(json["head"]["sequence"].asUInt64(), 5U);
    EXPECT_EQ(json["head"]["rule_id"].asUInt64(), 21U);
    EXPECT_EQ(json["head"]["event_name"].asString(), "EVENT_TYPE_EXEC");
    EXPECT_EQ(json["body"]["pid"].asInt(), 100);
    EXPECT_EQ(json["body"]["uid"].asUInt(), 501U);
    EXPECT_EQ(json["body"]["name"].asString(), "collector");
    EXPECT_EQ(json["body"]["path"].asString(), "/usr/bin/collector");
    EXPECT_EQ(json["body"]["args"].asString(), "--run");
}

TEST(SchemaHelperTest, SerializesNetworkBodyAndRejectsInvalidMagic)
{
    SCHEMA schema{};
    ASSERT_EQ(schema_init(&schema, sizeof(schema)), RC_OK);
    schema.Head.event_type = EVENT_TYPE_CONNECT;
    schema.Body.connect.protocol = 17;
    schema.Body.connect.src_port = 1234;
    schema.Body.connect.dst_port = 53;
    schema.Body.connect.is_ipv6 = 1;
    std::strcpy(reinterpret_cast<char*>(schema.Body.connect.src_addr), "2001:db8::1");
    std::strcpy(reinterpret_cast<char*>(schema.Body.connect.dst_addr), "2001:db8::53");

    Json::Value json;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(schema2string(&schema), json));
    EXPECT_EQ(json["head"]["event_name"].asString(), "EVENT_TYPE_CONNECT");
    EXPECT_EQ(json["body"]["protocol"].asUInt64(), 17U);
    EXPECT_EQ(json["body"]["src_port"].asUInt(), 1234U);
    EXPECT_EQ(json["body"]["dst_port"].asUInt(), 53U);
    EXPECT_TRUE(json["body"]["is_ipv6"].asBool());
    EXPECT_EQ(json["body"]["src_addr"].asString(), "2001:db8::1");
    EXPECT_EQ(json["body"]["dst_addr"].asString(), "2001:db8::53");

    schema.Head.magic[0] = 'X';
    EXPECT_EQ(schema_check(&schema), RC_FAIL);
    EXPECT_TRUE(schema2string(static_cast<const SCHEMA*>(nullptr)).empty());
    EXPECT_TRUE(schema2string(&schema.Head, nullptr).empty());
}

} // namespace
