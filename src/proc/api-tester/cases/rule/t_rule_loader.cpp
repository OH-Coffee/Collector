#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "file/file_api.hpp"
#include "filter/filter__executor.hpp"
#include "rule/loader.hpp"
#include "../file/file_test_utils.hpp"

namespace {

TEST(RuleLoaderTest, LoadsGroupsRulesAndConditionsFromXml)
{
    rule::Loader loader;
    ASSERT_EQ(loader.load_by_data(
                  R"(<FilterRules>
                    <EventRuleGroup ID="8" EventID="900">
                      <Rule ID="20" Action="2" Relationship="1">
                        <Condition ID="2" Object="3" Field="4" Matcher="5">alpha|beta</Condition>
                        <Condition ID="2" Object="3" Field="4" Matcher="5">delta</Condition>
                        <Condition ID="2" Object="3" Feild="4" Matcher="9">separate</Condition>
                        <Condition ID="1" Object="6" Field="7" Matcher="8">gamma</Condition>
                      </Rule>
                      <Rule ID="10" Action="1" Relationship="2" />
                    </EventRuleGroup>
                    <EventRuleGroup ID="9" EventID="100">
                      <Rule ID="30" Action="1" Relationship="2" />
                    </EventRuleGroup>
                  </FilterRules>)",
                  rule::MODE_RELOAD),
              RC_OK);

    const rule::Rules& groups = loader.get_filter_rule_groups();
    ASSERT_EQ(groups.size(), 2U);
    ASSERT_NE(groups.find(900), groups.end());
    ASSERT_NE(groups.find(100), groups.end());

    const rule::GroupPtr& group = groups.at(900);
    ASSERT_EQ(group->id, 8);
    ASSERT_EQ(group->event, 900U);
    ASSERT_EQ(group->rules.size(), 2U);

    auto rule_iterator = group->rules.begin();
    ASSERT_EQ((*rule_iterator)->id, 10);
    EXPECT_EQ((*rule_iterator)->action, filter::ACTION_INCLUDE);
    EXPECT_EQ((*rule_iterator)->relationship, filter::RELATIONSHIP_OR);
    EXPECT_TRUE((*rule_iterator)->conditions.empty());

    ++rule_iterator;
    ASSERT_EQ((*rule_iterator)->id, 20);
    EXPECT_EQ((*rule_iterator)->action, filter::ACTION_EXCLUDE);
    EXPECT_EQ((*rule_iterator)->relationship, filter::RELATIONSHIP_AND);
    ASSERT_EQ((*rule_iterator)->conditions.size(), 3U);

    auto condition_iterator = (*rule_iterator)->conditions.begin();
    ASSERT_EQ((*condition_iterator)->id, 1);
    EXPECT_EQ((*condition_iterator)->object, 6);
    EXPECT_EQ((*condition_iterator)->feild, 7);
    EXPECT_EQ((*condition_iterator)->matcher, 8);
    EXPECT_EQ(std::vector<std::string>((*condition_iterator)->patterns.begin(),
                                       (*condition_iterator)->patterns.end()),
              (std::vector<std::string>{"gamma"}));

    ++condition_iterator;
    ASSERT_EQ((*condition_iterator)->id, 2);
    EXPECT_EQ((*condition_iterator)->object, 3);
    EXPECT_EQ((*condition_iterator)->feild, 4);
    EXPECT_EQ((*condition_iterator)->matcher, 5);
    EXPECT_EQ(std::vector<std::string>((*condition_iterator)->patterns.begin(),
                                       (*condition_iterator)->patterns.end()),
              (std::vector<std::string>{"alpha", "beta", "delta"}));

    ++condition_iterator;
    ASSERT_EQ((*condition_iterator)->id, 2);
    EXPECT_EQ((*condition_iterator)->feild, 4);
    EXPECT_EQ((*condition_iterator)->matcher, 9);
    EXPECT_EQ(std::vector<std::string>((*condition_iterator)->patterns.begin(),
                                       (*condition_iterator)->patterns.end()),
              (std::vector<std::string>{"separate"}));

    EXPECT_EQ(groups.begin()->first, 100U);
}

TEST(RuleLoaderTest, RejectsMalformedXmlAndMissingRoot)
{
    rule::Loader loader;
    ASSERT_EQ(loader.load_by_data(
                  R"(<FilterRules><EventRuleGroup ID="1" EventID="42" /></FilterRules>)",
                  rule::MODE_RELOAD),
              RC_OK);

    EXPECT_EQ(loader.load_by_data(R"(<FilterRules>)", rule::MODE_RELOAD),
              RC_XML_PARSE_ERR);
    ASSERT_EQ(loader.get_filter_rule_groups().size(), 1U);
    EXPECT_EQ(loader.load_by_data(R"(<OtherRules />)", rule::MODE_RELOAD),
              RC_XML_PARSE_ERR);
    EXPECT_EQ(loader.get_filter_rule_groups().size(), 1U);
}

TEST(RuleLoaderTest, ReloadsValidXmlAndUnloadsGroups)
{
    rule::Loader loader;
    ASSERT_EQ(loader.load_by_data(
                  R"(<FilterRules><EventRuleGroup ID="1" EventID="42" /></FilterRules>)",
                  rule::MODE_RELOAD),
              RC_OK);
    ASSERT_EQ(loader.get_filter_rule_groups().size(), 1U);

    ASSERT_EQ(loader.load_by_data(R"(<FilterRules />)", rule::MODE_RELOAD), RC_OK);
    EXPECT_TRUE(loader.get_filter_rule_groups().empty());

    ASSERT_EQ(loader.load_by_data(
                  R"(<FilterRules><EventRuleGroup ID="2" EventID="43" /></FilterRules>)",
                  rule::MODE_RELOAD),
              RC_OK);
    EXPECT_EQ(loader.unload(), RC_OK);
    EXPECT_TRUE(loader.get_filter_rule_groups().empty());
}

TEST(RuleLoaderTest, LoadsXmlFromPath)
{
    file_test::TemporaryDirectory test_directory;
    const std::string rule_path = test_directory.path("rules.xml");
    ASSERT_EQ(file_write(
                  rule_path,
                  R"(<FilterRules><EventRuleGroup ID="7" EventID="77" /></FilterRules>)"),
              std::string(
                  R"(<FilterRules><EventRuleGroup ID="7" EventID="77" /></FilterRules>)")
                  .size());

    rule::Loader loader;
    ASSERT_EQ(loader.load_by_path(rule_path, rule::MODE_RELOAD), RC_OK);
    ASSERT_EQ(loader.get_filter_rule_groups().size(), 1U);
    EXPECT_EQ(loader.get_filter_rule_groups().at(77)->id, 7);
    EXPECT_EQ(loader.print(),
              "<FilterRules>\n"
              "\t<EventRuleGroup ID=\"7\" EventID=\"77\"/>\n"
              "</FilterRules>\n");
}

TEST(RuleLoaderTest, PrintSerializesStoredRuleData)
{
    const std::string xml =
        R"(<FilterRules Version="1"><EventRuleGroup ID="3" EventID="33">)"
        R"(<Rule ID="4" Relationship="1" Action="2">)"
        R"(<Condition ID="5" Object="6" Field="7" Matcher="8">A&amp;B|x&lt;y)"
        R"(</Condition></Rule></EventRuleGroup></FilterRules>)";
    rule::Loader loader;

    ASSERT_EQ(loader.load_by_data(xml, rule::MODE_RELOAD), RC_OK);
    auto& group = loader.get_filter_rule_groups().at(33);
    EXPECT_EQ(group->rules.front()->conditions.front()->feild, 7);
    EXPECT_EQ(std::vector<std::string>(
                  group->rules.front()->conditions.front()->patterns.begin(),
                  group->rules.front()->conditions.front()->patterns.end()),
              (std::vector<std::string>{"A&B", "x<y"}));

    group->id = 30;
    group->rules.front()->conditions.front()->patterns.front() = "C&D";
    const std::string expected_xml =
        "<FilterRules>\n"
        "\t<EventRuleGroup ID=\"30\" EventID=\"33\">\n"
        "\t\t<Rule ID=\"4\" Action=\"2\" Relationship=\"1\">\n"
        "\t\t\t<Condition ID=\"5\" Object=\"6\" Field=\"7\" Matcher=\"8\">"
        "C&amp;D|x&lt;y</Condition>\n"
        "\t\t</Rule>\n"
        "\t</EventRuleGroup>\n"
        "</FilterRules>\n";
    EXPECT_EQ(loader.print(), expected_xml);

    rule::Loader round_trip_loader;
    ASSERT_EQ(round_trip_loader.load_by_data(loader.print(), rule::MODE_RELOAD), RC_OK);
    EXPECT_EQ(round_trip_loader.get_filter_rule_groups().at(33)->id, 30);
    EXPECT_EQ(std::vector<std::string>(
                  round_trip_loader.get_filter_rule_groups()
                      .at(33)->rules.front()->conditions.front()->patterns.begin(),
                  round_trip_loader.get_filter_rule_groups()
                      .at(33)->rules.front()->conditions.front()->patterns.end()),
              (std::vector<std::string>{"C&D", "x<y"}));

    EXPECT_EQ(loader.load_by_data(R"(<FilterRules>)", rule::MODE_RELOAD),
              RC_XML_PARSE_ERR);

    EXPECT_EQ(loader.unload(), RC_OK);
    EXPECT_EQ(loader.print(), "<FilterRules/>\n");
}

TEST(RuleLoaderTest, TogglesDefaultOnMatch)
{
    rule::Loader loader;

    EXPECT_FALSE(loader.enable_default_on_match());
    loader.enable_default_on_match(true);
    EXPECT_TRUE(loader.enable_default_on_match());
    loader.enable_default_on_match(false);
    EXPECT_FALSE(loader.enable_default_on_match());
}

TEST(RuleStructTest, ClearResetsConditionRuleAndGroup)
{
    rule::Condition condition;
    condition.id = 1;
    condition.object = 2;
    condition.feild = 3;
    condition.matcher = 4;
    condition.patterns.emplace_back("pattern");
    condition.clear();
    EXPECT_EQ(condition.id, 0);
    EXPECT_EQ(condition.object, -1);
    EXPECT_EQ(condition.feild, -1);
    EXPECT_EQ(condition.matcher, 0);
    EXPECT_TRUE(condition.patterns.empty());

    rule::Rule rule_value;
    rule_value.id = 1;
    rule_value.action = filter::ACTION_INCLUDE;
    rule_value.relationship = filter::RELATIONSHIP_OR;
    rule_value.conditions.emplace_back(rule::Condition::create());
    rule_value.clear();
    EXPECT_EQ(rule_value.id, -1);
    EXPECT_EQ(rule_value.action, filter::ACTION_UNKNOWN);
    EXPECT_EQ(rule_value.relationship, filter::RELATIONSHIP_UNKNOWN);
    EXPECT_TRUE(rule_value.conditions.empty());

    rule::Group group;
    group.id = 1;
    group.event = 2;
    group.rules.emplace_back(rule::Rule::create());
    group.clear();
    EXPECT_EQ(group.id, -1);
    EXPECT_EQ(group.event, 0U);
    EXPECT_TRUE(group.rules.empty());
}

} // namespace
