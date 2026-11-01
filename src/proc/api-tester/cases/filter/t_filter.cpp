#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "filter/filter.hpp"

namespace {

std::string make_condition(int id, int object, int field, filter::MatchMode matcher,
                           const std::string& pattern)
{
    return "<Condition ID=\"" + std::to_string(id) + "\" Object=\"" +
           std::to_string(object) + "\" Field=\"" + std::to_string(field) +
           "\" Matcher=\"" + std::to_string(matcher) + "\">" + pattern +
           "</Condition>";
}

std::string make_rule(int id, filter::Action action, filter::Relationship relationship,
                      const std::vector<std::string>& conditions)
{
    std::string xml = "<Rule ID=\"" + std::to_string(id) + "\" Action=\"" +
                      std::to_string(action) + "\" Relationship=\"" +
                      std::to_string(relationship) + "\">";
    for (const auto& condition : conditions) {
        xml += condition;
    }
    return xml + "</Rule>";
}

std::string make_group(int id, uint64_t event,
                       const std::vector<std::string>& rules)
{
    std::string xml = "<EventRuleGroup ID=\"" + std::to_string(id) +
                      "\" EventID=\"" + std::to_string(event) + "\">";
    for (const auto& rule : rules) {
        xml += rule;
    }
    return xml + "</EventRuleGroup>";
}

void load_filter(filter::SubFilter& target, const std::string& groups)
{
    rule::Loader loader;
    const std::string xml = "<FilterRules>" + groups + "</FilterRules>";
    ASSERT_EQ(loader.load_by_data(xml, rule::MODE_RELOAD), RC_OK);
    ASSERT_EQ(target.load(loader), RC_OK);
}

filter::StructSet process_data(const std::string& name,
                               const std::string& path = "/usr/bin/agent")
{
    filter::StructSet data;
    data.proc = std::make_shared<ProcInfo>();
    data.proc->name = name;
    data.proc->path = path;
    return data;
}

TEST(FilterTest, FieldIdentifiersAreAvailable)
{
    static_assert(filter::OID__PROCESS == 101);
    static_assert(filter::FID_PROC__PID == 1001);
    SUCCEED();
}

TEST(FilterTest, ExtractsMappedFieldsAndReturnsEmptyForUnknownFields)
{
    auto data = process_data("agent", "/usr/local/bin/agent");
    data.proc->pid = 42;

    auto pid_condition = rule::Condition::create();
    pid_condition->object = filter::OID__PROCESS;
    pid_condition->feild = filter::FID_PROC__PID;
    EXPECT_EQ(filter::new_extractor(pid_condition)(data), "42");

    auto file_name_condition = rule::Condition::create();
    file_name_condition->object = filter::OID__PROCESS_FILE;
    file_name_condition->feild = filter::FID_FILE__NAME;
    EXPECT_EQ(filter::new_extractor(file_name_condition)(data), "agent");

    auto unknown_condition = rule::Condition::create();
    unknown_condition->object = filter::OID__UNKNOWN;
    unknown_condition->feild = filter::FID_UNKNOWN;
    EXPECT_TRUE(filter::new_extractor(unknown_condition)(data).empty());
}

TEST(FilterTest, EvaluatesSupportedStringMatchers)
{
    struct MatcherCase
    {
        filter::MatchMode matcher;
        int object;
        int field;
        std::string patterns;
        std::string matching_name;
        std::string matching_path;
        std::string nonmatching_name;
        std::string nonmatching_path;
    };

    const std::vector<MatcherCase> cases = {
        {filter::MATCH_MODE__IS, filter::OID__PROCESS, filter::FID_PROC__NAME,
         "agent", "agent", "", "xagent", ""},
        {filter::MATCH_MODE__IS_ANY, filter::OID__PROCESS, filter::FID_PROC__NAME,
         "agent|worker", "worker", "", "xagent", ""},
        {filter::MATCH_MODE__CONTAINS, filter::OID__PROCESS, filter::FID_PROC__NAME,
         "gent", "agent", "", "worker", ""},
        {filter::MATCH_MODE__CONTAINS_ANY, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "gent|work", "worker", "", "daemon", ""},
        {filter::MATCH_MODE__NOT, filter::OID__PROCESS, filter::FID_PROC__NAME,
         "agent", "worker", "", "agent", ""},
        {filter::MATCH_MODE__NOT_ANY, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent|worker", "daemon", "", "worker", ""},
        {filter::MATCH_MODE__NOT_CONTAINS, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent", "worker", "", "agent", ""},
        {filter::MATCH_MODE__BEGIN_WITH, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent", "agentd", "", "myagent", ""},
        {filter::MATCH_MODE__BEGIN_WITH_ANY, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent|work", "worker", "", "myagent", ""},
        {filter::MATCH_MODE__END_WITH, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent", "myagent", "", "agentd", ""},
        {filter::MATCH_MODE__END_WITH_ANY, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent|worker", "myworker", "", "worker1", ""},
        {filter::MATCH_MODE__IS_ANY__CT, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent|worker", "worker", "", "xagent", ""},
        {filter::MATCH_MODE__BEGIN_WITH_ANY__CT, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent|worker", "workerd", "", "myagent", ""},
        {filter::MATCH_MODE__BEGIN_WITH_ANY__PT, filter::OID__PROCESS_FILE,
         filter::FID_FILE__PATH, "/usr/bin|/opt/bin", "",
         "/usr/bin/collector", "", "/usr/sbin/collector"},
        {filter::MATCH_MODE__END_WITH_ANY__CT, filter::OID__PROCESS,
         filter::FID_PROC__NAME, "agent|worker", "myworker", "", "daemon", ""},
        {filter::MATCH_MODE__END_WITH_ANY__PT, filter::OID__PROCESS_FILE,
         filter::FID_FILE__PATH, "/usr/bin|/opt/bin", "",
         "/tmp/usr/bin", "", "/tmp/usr/sbin"},
    };

    uint64_t event = 10000;
    for (const auto& test_case : cases) {
        SCOPED_TRACE(test_case.matcher);
        filter::SubFilter subfilter;
        const auto condition = make_condition(
            3, test_case.object, test_case.field, test_case.matcher,
            test_case.patterns);
        load_filter(
            subfilter,
            make_group(7, event, {make_rule(9, filter::ACTION_INCLUDE,
                                           filter::RELATIONSHIP_OR, {condition})}));

        const auto matching = process_data(test_case.matching_name,
                                           test_case.matching_path.empty()
                                               ? "/usr/bin/agent"
                                               : test_case.matching_path);
        const auto nonmatching = process_data(test_case.nonmatching_name,
                                              test_case.nonmatching_path.empty()
                                                  ? "/usr/bin/agent"
                                                  : test_case.nonmatching_path);
        EXPECT_EQ(subfilter.match(event, matching),
                  RC_FILTER_MATCH_INCLUEDE);
        EXPECT_EQ(subfilter.match(event, nonmatching), RC_FILTER_MATCH_UNKNOWN);
        ++event;
    }
}

TEST(FilterTest, CombinesConditionsAndAppliesRulesInIdOrder)
{
    constexpr uint64_t event = 7101;
    filter::SubFilter subfilter;
    const std::string and_rule = make_rule(
        10, filter::ACTION_EXCLUDE, filter::RELATIONSHIP_AND,
        {make_condition(1, filter::OID__PROCESS, filter::FID_PROC__NAME,
                        filter::MATCH_MODE__IS, "agent"),
         make_condition(2, filter::OID__PROCESS_FILE, filter::FID_FILE__PATH,
                        filter::MATCH_MODE__BEGIN_WITH, "/srv")});
    const std::string or_rule = make_rule(
        20, filter::ACTION_INCLUDE, filter::RELATIONSHIP_OR,
        {make_condition(1, filter::OID__PROCESS, filter::FID_PROC__NAME,
                        filter::MATCH_MODE__IS_ANY, "agent|worker"),
         make_condition(2, filter::OID__PROCESS_FILE, filter::FID_FILE__PATH,
                        filter::MATCH_MODE__BEGIN_WITH, "/opt")});
    load_filter(subfilter, make_group(71, event, {or_rule, and_rule}));

    std::string id;
    EXPECT_EQ(subfilter.match(event, process_data("agent"), id),
              RC_FILTER_MATCH_INCLUEDE);
    EXPECT_EQ(id, "71-20-1");

    id.clear();
    EXPECT_EQ(subfilter.match(event, process_data("agent", "/srv/bin/agent"), id),
              RC_FILTER_MATCH_EXCLUEDE);
    EXPECT_EQ(id, "71-10");
}

TEST(FilterTest, IsolatesEventsAndUsesDefaultActionOnlyWhenEnabled)
{
    constexpr uint64_t unmatched_exclude_event = 7201;
    constexpr uint64_t unmatched_include_event = 7202;
    constexpr uint64_t no_default_event = 7203;
    filter::SubFilter subfilter;
    load_filter(
        subfilter,
        make_group(81, unmatched_exclude_event,
                   {make_rule(1, filter::ACTION_EXCLUDE, filter::RELATIONSHIP_OR,
                              {make_condition(1, filter::OID__PROCESS,
                                              filter::FID_PROC__NAME,
                                              filter::MATCH_MODE__IS, "blocked")})}) +
            make_group(82, unmatched_include_event,
                       {make_rule(1, filter::ACTION_INCLUDE,
                                  filter::RELATIONSHIP_OR,
                                  {make_condition(1, filter::OID__PROCESS,
                                                  filter::FID_PROC__NAME,
                                                  filter::MATCH_MODE__IS,
                                                  "allowed")})}) +
            make_group(83, no_default_event,
                       {make_rule(1, filter::ACTION_EXCLUDE,
                                  filter::RELATIONSHIP_OR,
                                  {make_condition(1, filter::OID__PROCESS,
                                                  filter::FID_PROC__NAME,
                                                  filter::MATCH_MODE__IS,
                                                  "blocked")})}));

    const auto data = process_data("agent");
    EXPECT_EQ(subfilter.match(unmatched_exclude_event, data),
              RC_FILTER_MATCH_UNKNOWN);
    subfilter.enable_default_on_match(true);
    EXPECT_EQ(subfilter.match(unmatched_exclude_event, data),
              RC_FILTER_MATCH_INCLUEDE);
    EXPECT_EQ(subfilter.match(unmatched_include_event, data),
              RC_FILTER_MATCH_EXCLUEDE);
    EXPECT_EQ(subfilter.match(unmatched_exclude_event + 100, data),
              RC_FILTER_MATCH_UNKNOWN);
    EXPECT_EQ(subfilter.match(unmatched_exclude_event, process_data("blocked")),
              RC_FILTER_MATCH_EXCLUEDE);

    filter::SubFilter no_default_filter;
    load_filter(no_default_filter,
                make_group(84, no_default_event,
                           {make_rule(1, filter::ACTION_EXCLUDE,
                                      filter::RELATIONSHIP_OR,
                                      {make_condition(1, filter::OID__PROCESS,
                                                      filter::FID_PROC__NAME,
                                                      filter::MATCH_MODE__IS,
                                                      "blocked")})}));
    EXPECT_EQ(no_default_filter.match(no_default_event, data),
              RC_FILTER_MATCH_UNKNOWN);
}

TEST(FilterTest, ReloadReplacesThePreviousRuleGroups)
{
    constexpr uint64_t event = 7250;
    filter::SubFilter subfilter;

    rule::Loader first_loader;
    ASSERT_EQ(first_loader.load_by_data(
                  "<FilterRules>" +
                      make_group(85, event,
                                 {make_rule(1, filter::ACTION_INCLUDE,
                                            filter::RELATIONSHIP_OR,
                                            {make_condition(
                                                1, filter::OID__PROCESS,
                                                filter::FID_PROC__NAME,
                                                filter::MATCH_MODE__IS, "old")})}) +
                      "</FilterRules>",
                  rule::MODE_RELOAD),
              RC_OK);
    ASSERT_EQ(subfilter.load(first_loader), RC_OK);
    EXPECT_EQ(subfilter.match(event, process_data("old")),
              RC_FILTER_MATCH_INCLUEDE);

    rule::Loader second_loader;
    ASSERT_EQ(second_loader.load_by_data(
                  "<FilterRules>" +
                      make_group(86, event,
                                 {make_rule(2, filter::ACTION_INCLUDE,
                                            filter::RELATIONSHIP_OR,
                                            {make_condition(
                                                1, filter::OID__PROCESS,
                                                filter::FID_PROC__NAME,
                                                filter::MATCH_MODE__IS, "new")})}) +
                      "</FilterRules>",
                  rule::MODE_RELOAD),
              RC_OK);
    ASSERT_EQ(subfilter.reload(second_loader), RC_OK);

    EXPECT_EQ(subfilter.match(event, process_data("old")),
              RC_FILTER_MATCH_UNKNOWN);
    EXPECT_EQ(subfilter.match(event, process_data("new")),
              RC_FILTER_MATCH_INCLUEDE);
}

TEST(FilterTest, FilterWrapperPropagatesLoaderDefaultSetting)
{
    constexpr uint64_t event = 7301;
    rule::Loader loader;
    ASSERT_EQ(loader.load_by_data(
                  "<FilterRules>" +
                      make_group(91, event,
                                 {make_rule(1, filter::ACTION_EXCLUDE,
                                            filter::RELATIONSHIP_OR,
                                            {make_condition(
                                                1, filter::OID__PROCESS,
                                                filter::FID_PROC__NAME,
                                                filter::MATCH_MODE__IS, "blocked")})}) +
                      "</FilterRules>",
                  rule::MODE_RELOAD),
              RC_OK);
    loader.enable_default_on_match(true);

    filter::Filter filter;
    ASSERT_EQ(filter.load(loader), RC_OK);
    EXPECT_EQ(filter.match(event, process_data("agent")),
              RC_FILTER_MATCH_INCLUEDE);
    EXPECT_EQ(filter.match(event + 1, process_data("agent")),
              RC_FILTER_MATCH_UNKNOWN);
}

} // namespace
