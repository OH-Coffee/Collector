#include <algorithm>
#include <vector>

#include "string/string_version.hpp"
#include "string/string_convert.hpp"
#include "string/string_split.hpp"

int string_version_compare(const std::string &i_v1, const std::string &i_v2, uint64_t i_place)
{
    const auto items_v1 = string_split<std::vector<std::string>>(i_v1, ".");
    const auto items_v2 = string_split<std::vector<std::string>>(i_v2, ".");

    uint64_t place_num = std::max(items_v1.size(), items_v2.size());
    if(i_place > 0) {
        place_num = std::min(place_num, i_place);
    }

    for(uint64_t i = 0; i < place_num; ++i) {
        const auto num1 = (i < items_v1.size()) ? string_to_integer<int>(items_v1[i]) : 0;
        const auto num2 = (i < items_v2.size()) ? string_to_integer<int>(items_v2[i]) : 0;

        if(num1 != num2) { return num1 - num2; }
    }

    return 0;
}
