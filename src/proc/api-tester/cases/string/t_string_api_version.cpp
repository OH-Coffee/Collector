#include <string>

#include "gtest/gtest.h"
#include "string/string_api.hpp"

namespace {

TEST(StringApiVersionTest, ComparesVersionComponentsNumerically)
{
    EXPECT_EQ(string_version_compare("1.2.3", "1.2.3"), 0);
    EXPECT_LT(string_version_compare("1.2.3", "1.2.10"), 0);
    EXPECT_GT(string_version_compare("2.0", "1.9.9"), 0);
}

TEST(StringApiVersionTest, TreatsMissingComponentsAsZero)
{
    EXPECT_EQ(string_version_compare("1.2", "1.2.0"), 0);
    EXPECT_LT(string_version_compare("1.2", "1.2.1"), 0);
}

TEST(StringApiVersionTest, LimitsComparisonToRequestedPlace)
{
    EXPECT_EQ(string_version_compare("1.2.3", "1.2.9", 2), 0);
    EXPECT_LT(string_version_compare("1.2.3", "1.2.9", 3), 0);
}

} // namespace
