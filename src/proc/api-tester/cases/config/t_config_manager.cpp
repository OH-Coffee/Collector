#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "config/config_manager.hpp"
#include "file/file_api.hpp"
#include "../file/file_test_utils.hpp"

namespace {

TEST(ConfigManagerTest, LoadsJsonDataAndReportsMalformedJson)
{
    ConfigManager config;

    EXPECT_EQ(config.load_by_data(
                  R"({"name":"Collector","enabled":true,"server":{"port":8080}})"),
              RC_OK);
    EXPECT_EQ(config.get_string("name"), "Collector");
    EXPECT_TRUE(config.get_bool("enabled"));
    EXPECT_EQ(config.get_int32("server.port"), 8080);

    EXPECT_EQ(config.load_by_data(R"({"name":"Collector",)"),
              RC_JSON_SERIALIZE_ERR);
}

TEST(ConfigManagerTest, LoadsJsonFromPathAndReportsUnreadableFiles)
{
    file_test::TemporaryDirectory test_directory;
    const std::string config_path = test_directory.path("config.json");
    ASSERT_EQ(file_write(config_path, R"({"name":"from-file"})"),
              std::string(R"({"name":"from-file"})").size());

    ConfigManager config;
    EXPECT_EQ(config.load(config_path), RC_OK);
    EXPECT_EQ(config.get_string("name"), "from-file");
    EXPECT_EQ(config.load_by_path(test_directory.path("missing.json")),
              RC_FILE_READ_ERR);

    const std::string empty_path = test_directory.path("empty.json");
    ASSERT_EQ(file_write(empty_path, ""), 0U);
    EXPECT_EQ(config.load_by_path(empty_path), RC_FILE_READ_ERR);
}

TEST(ConfigManagerTest, GetsTypedValuesAndReturnsDefaultsForMissingOrWrongTypes)
{
    ConfigManager config;
    ASSERT_EQ(config.load_by_data(
                  R"({"int32":-42,"int64":4294967296,"uint32":42,"uint64":18446744073709551615,"enabled":true,"name":"Collector","nested":{"value":7},"wrong":"type"})"),
              RC_OK);

    EXPECT_EQ(config.get_int32("int32"), -42);
    EXPECT_EQ(config.get_int64("int64"), 4294967296LL);
    EXPECT_EQ(config.get_uint32("uint32"), 42U);
    EXPECT_EQ(config.get_uint64("uint64"), UINT64_MAX);
    EXPECT_TRUE(config.get_bool("enabled"));
    EXPECT_EQ(config.get_string("name"), "Collector");
    EXPECT_EQ(config.get_int32("nested.value"), 7);

    EXPECT_EQ(config.get_int32("missing"), -1);
    EXPECT_EQ(config.get_int64("wrong"), -1);
    EXPECT_EQ(config.get_uint32("wrong"), 0U);
    EXPECT_EQ(config.get_uint64("missing"), 0U);
    EXPECT_FALSE(config.get_bool("wrong"));
    EXPECT_TRUE(config.get_string("missing").empty());

    Json::Value serialized_value;
    Json::Reader reader;
    ASSERT_TRUE(reader.parse(config.to_string(), serialized_value));
    EXPECT_FALSE(serialized_value.isMember("missing"));
}

TEST(ConfigManagerTest, SerializesCompactAndStyledJson)
{
    ConfigManager config;
    ASSERT_EQ(config.load_by_data(R"({"name":"Collector","nested":{"enabled":true}})"),
              RC_OK);

    const std::string compact_json = config.to_string();
    const std::string styled_json = config.to_string(true);
    Json::Value compact_value;
    Json::Value styled_value;
    Json::Reader reader;

    ASSERT_TRUE(reader.parse(compact_json, compact_value));
    ASSERT_TRUE(reader.parse(styled_json, styled_value));
    EXPECT_EQ(compact_value["name"].asString(), "Collector");
    EXPECT_TRUE(styled_value["nested"]["enabled"].asBool());
    EXPECT_NE(styled_json.find('\n'), std::string::npos);
}

TEST(ConfigManagerTest, SetsNestedValuesAndArraysWithOptionalRewrite)
{
    ConfigManager config;
    config.set_value("service.port", 8080);
    config.set_value("service.port", 9090);
    EXPECT_EQ(config.get_int32("service.port"), 8080);

    config.set_value("service.port", 9090, nullptr, true);
    EXPECT_EQ(config.get_int32("service.port"), 9090);

    config.set_array("service.tags", std::vector<std::string>{"stable", "fast"});
    config.set_array("service.tags", std::vector<std::string>{"ignored"});
    EXPECT_EQ(config.get_strings<std::vector<std::string>>("service.tags"),
              (std::vector<std::string>{"stable", "fast"}));

    config.set_array("service.tags", std::vector<std::string>{"new"}, nullptr, true);
    EXPECT_EQ(config.get_strings<std::vector<std::string>>("service.tags"),
              (std::vector<std::string>{"new"}));

    Json::Value other_root;
    config.set_value("custom.enabled", true, &other_root);
    EXPECT_TRUE(other_root["custom"]["enabled"].asBool());
    EXPECT_FALSE(config.get_bool("custom.enabled"));
}

TEST(ConfigManagerTest, GetsOnlyStringElementsFromArrays)
{
    ConfigManager config;
    ASSERT_EQ(config.load_by_data(R"({"values":["first",1,true,"second"],"not_array":"value"})"),
              RC_OK);

    EXPECT_EQ(config.get_strings<std::vector<std::string>>("values"),
              (std::vector<std::string>{"first", "second"}));
    EXPECT_TRUE(config.get_strings<std::vector<std::string>>("not_array").empty());
    EXPECT_TRUE(config.get_strings<std::vector<std::string>>("missing").empty());
}

} // namespace
