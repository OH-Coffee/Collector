#include <set>
#include <string>

#include "gtest/gtest.h"
#include "user/user_info.hpp"

namespace {

TEST(UserInfoTest, ReturnsUniqueUserNames)
{
    const std::set<std::string> all_user_names = user_names();

    EXPECT_EQ(all_user_names.size(), user_names().size());
    for (const auto& user_name : all_user_names) {
        EXPECT_FALSE(user_name.empty());
    }
}

TEST(UserInfoTest, LoginableNamesAreKnownUsers)
{
    const std::set<std::string> all_user_names = user_names();
    const std::set<std::string> loginable_user_names = user_names_loginable();

    for (const auto& user_name : loginable_user_names) {
        EXPECT_NE(all_user_names.count(user_name), 0U);
    }
}

TEST(UserInfoTest, NamesWithoutLeadingUnderscoresAreKnownUsers)
{
    const std::set<std::string> all_user_names = user_names();
    const std::set<std::string> filtered_user_names = user_names_no_underline();

    for (const auto& user_name : filtered_user_names) {
        EXPECT_FALSE(user_name.empty());
        EXPECT_NE(user_name.front(), '_');
        EXPECT_NE(all_user_names.count(user_name), 0U);
    }
}

TEST(UserInfoTest, ReturnsUniqueHomeDirectories)
{
    const std::set<std::string> home_dirs = user_home_dirs();

    EXPECT_EQ(home_dirs.size(), user_home_dirs().size());
}

TEST(UserInfoTest, FiltersHomeDirectoriesByPrefix)
{
    const std::set<std::string> all_home_dirs = user_home_dirs();
    const std::set<std::string> root_home_dirs = user_home_dirs_in_users("/");

    for (const auto& home_dir : root_home_dirs) {
        EXPECT_NE(home_dir.find('/'), std::string::npos);
        EXPECT_NE(all_home_dirs.count(home_dir), 0U);
    }

    const std::set<std::string> no_match = user_home_dirs_in_users(
        "/__collector_test_no_such_home_prefix__/");
    EXPECT_TRUE(no_match.empty());
}

} // namespace
