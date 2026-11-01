#include <string>
#include <sstream>

#include "gtest/gtest.h"
#include "json/json.h"

namespace {

TEST(JsonCppReaderTest, ParsesNestedJsonDocument)
{
    const std::string in_json =
        R"({"name":"Collector","enabled":true,"items":[1,2,3],"config":{"retry":4}})";
    Json::Value json_value;
    Json::Reader json_reader;

    ASSERT_TRUE(json_reader.parse(in_json, json_value));
    EXPECT_EQ(json_value["name"].asString(), "Collector");
    EXPECT_TRUE(json_value["enabled"].asBool());
    ASSERT_EQ(json_value["items"].size(), 3U);
    EXPECT_EQ(json_value["items"][1U].asInt(), 2);
    EXPECT_EQ(json_value["config"]["retry"].asInt(), 4);
}

TEST(JsonCppReaderTest, ReportsMalformedJson)
{
    const std::string in_json = R"({"name": "Collector", "items": [1, 2)";
    Json::Value json_value;
    Json::Reader json_reader;

    EXPECT_FALSE(json_reader.parse(in_json, json_value));
    EXPECT_FALSE(json_reader.getFormattedErrorMessages().empty());
}

TEST(JsonCppReaderTest, ParsesJsonFromInputStream)
{
    std::istringstream in_stream(R"({"status":"ready"})");
    Json::Value json_value;
    Json::Reader json_reader;

    ASSERT_TRUE(json_reader.parse(in_stream, json_value));
    EXPECT_EQ(json_value["status"].asString(), "ready");
}

} // namespace
