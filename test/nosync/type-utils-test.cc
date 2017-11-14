// This file is part of libnosync library. See LICENSE file for license details.
#include <cstdint>
#include <gtest/gtest.h>
#include <nosync/type-utils.h>

using nosync::sizeof_in_atoms;
using nosync::sizeof_in_bits;
using nosync::sizeof_sum;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;


TEST(NosyncTypeUtils, SizeofInAtoms) {
    ASSERT_EQ((sizeof_in_atoms<char, char>), 1U);
    ASSERT_EQ((sizeof_in_atoms<signed char, unsigned char>), 1U);

    ASSERT_EQ((sizeof_in_atoms<uint64_t, uint64_t>), 1U);
    ASSERT_EQ((sizeof_in_atoms<uint32_t, uint64_t>), 2U);
    ASSERT_EQ((sizeof_in_atoms<uint16_t, uint64_t>), 4U);
    ASSERT_EQ((sizeof_in_atoms<uint8_t, uint64_t>), 8U);
    ASSERT_EQ((sizeof_in_atoms<char, uint64_t>), 8U);

    ASSERT_EQ((sizeof_in_atoms<int32_t, int32_t>), 1U);
    ASSERT_EQ((sizeof_in_atoms<int16_t, int32_t>), 2U);
    ASSERT_EQ((sizeof_in_atoms<int8_t, int32_t>), 4U);
    ASSERT_EQ((sizeof_in_atoms<char, int32_t>), 4U);
}


TEST(NosyncTypeUtils, SizeofInBits) {
    ASSERT_EQ(sizeof_in_bits<uint64_t>, 64U);
    ASSERT_EQ(sizeof_in_bits<uint32_t>, 32U);
    ASSERT_EQ(sizeof_in_bits<uint16_t>, 16U);
    ASSERT_EQ(sizeof_in_bits<uint8_t>, 8U);

    ASSERT_EQ(sizeof_in_bits<int64_t>, 64U);
    ASSERT_EQ(sizeof_in_bits<int32_t>, 32U);
    ASSERT_EQ(sizeof_in_bits<int16_t>, 16U);
    ASSERT_EQ(sizeof_in_bits<int8_t>, 8U);
}


TEST(NosyncTypeUtils, SizeofSum) {
    ASSERT_EQ((sizeof_sum<>), 0U);
    ASSERT_EQ((sizeof_sum<char>), 1U);
    ASSERT_EQ((sizeof_sum<uint32_t>), 4U);
    ASSERT_EQ((sizeof_sum<char, uint32_t, char>), 6U);
    ASSERT_EQ((sizeof_sum<uint16_t, char, uint64_t, uint32_t>), 15U);
}
