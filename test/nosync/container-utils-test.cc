// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <nosync/container-utils.h>
#include <string>

using namespace std::string_literals;
using nosync::generate_array;
using std::string;
using std::to_string;


TEST(NosyncContainerUtils, GenerateArrayWithStrings)
{
    auto str_values = generate_array<string, 3>(
        [](auto index) {
            return to_string(index);
        });

    ASSERT_EQ(str_values.size(), 3U);
    ASSERT_EQ(str_values[0], "0"s);
    ASSERT_EQ(str_values[1], "1"s);
    ASSERT_EQ(str_values[2], "2"s);
}
