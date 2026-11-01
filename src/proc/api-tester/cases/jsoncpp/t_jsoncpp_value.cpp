#include <string>

#include "gtest/gtest.h"
#include "json/json.h"

namespace {

TEST(JsonCppValueTest, RepresentsJsonValueTypes)
{
    const Json::Value null_value;
    const Json::Value bool_value(true);
    const Json::Value int_value(42);
    const Json::Value real_value(3.5);
    const Json::Value string_value("Collector");
    const Json::Value array_value(Json::arrayValue);
    const Json::Value object_value(Json::objectValue);

    EXPECT_TRUE(null_value.isNull());
    EXPECT_TRUE(bool_value.isBool());
    EXPECT_TRUE(int_value.isInt());
    EXPECT_TRUE(real_value.isDouble());
    EXPECT_TRUE(string_value.isString());
    EXPECT_TRUE(array_value.isArray());
    EXPECT_TRUE(object_value.isObject());
}

TEST(JsonCppValueTest, ReadsAndUpdatesObjectMembers)
{
    Json::Value json_value(Json::objectValue);
    json_value["name"] = "Collector";
    json_value["enabled"] = true;
    json_value["count"] = 3;

    EXPECT_TRUE(json_value.isMember("name"));
    EXPECT_EQ(json_value["name"].asString(), "Collector");
    EXPECT_TRUE(json_value["enabled"].asBool());
    EXPECT_EQ(json_value["count"].asInt(), 3);
    EXPECT_EQ(json_value.get("missing", Json::Value("default")).asString(), "default");
    EXPECT_EQ(json_value.size(), 3U);
}

TEST(JsonCppValueTest, AppendsAndResizesArrays)
{
    Json::Value json_value(Json::arrayValue);
    json_value.append("first");
    json_value.append(2);
    json_value.resize(3);

    ASSERT_EQ(json_value.size(), 3U);
    EXPECT_EQ(json_value[0U].asString(), "first");
    EXPECT_EQ(json_value[1U].asInt(), 2);
    EXPECT_TRUE(json_value[2U].isNull());
}

TEST(JsonCppValueTest, PreservesEmbeddedNullsInStrings)
{
    const std::string in_string{"a\0b", 3};
    const Json::Value json_value(in_string);

    EXPECT_EQ(json_value.asString(), in_string);
}

} // namespace
