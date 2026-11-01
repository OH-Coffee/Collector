#include <string>

#include "gtest/gtest.h"
#include "json/json.h"

namespace {

void expect_writer_round_trip(Json::Writer& i_writer)
{
    Json::Value in_value(Json::objectValue);
    in_value["name"] = "Collector";
    in_value["active"] = true;
    in_value["version"] = 2;
    in_value["tags"].append("unit");
    in_value["tags"].append("test");

    const std::string out_json = i_writer.write(in_value);
    Json::Value out_value;
    Json::Reader json_reader;

    ASSERT_TRUE(json_reader.parse(out_json, out_value)) << json_reader.getFormattedErrorMessages();
    EXPECT_EQ(out_value, in_value);
}

TEST(JsonCppWriterTest, FastWriterProducesParseableJson)
{
    Json::FastWriter json_writer;

    expect_writer_round_trip(json_writer);
}

TEST(JsonCppWriterTest, StyledWriterProducesParseableJson)
{
    Json::StyledWriter json_writer;

    expect_writer_round_trip(json_writer);
}

TEST(JsonCppWriterTest, FastWriterCanOmitEndingLineFeed)
{
    Json::FastWriter json_writer;
    json_writer.omitEndingLineFeed();

    const std::string out_json = json_writer.write(Json::Value("value"));

    ASSERT_FALSE(out_json.empty());
    EXPECT_NE(out_json.back(), '\n');
}

} // namespace
