// This file is part of libnosync library. See LICENSE file for license details.
#include <array>
#include <cstdint>
#include <experimental/array>
#include <gtest/gtest.h>
#include <nosync/number-utils.h>

using namespace std::string_literals;
using nosync::cast_to_unsigned;
using nosync::decode_be_atoms_to_number;
using nosync::decode_be_bytes_to_number;
using nosync::decode_le_atoms_to_number;
using nosync::decode_le_bytes_to_number;
using nosync::encode_to_be_atoms;
using nosync::encode_to_be_bytes;
using nosync::encode_to_le_atoms;
using nosync::encode_to_le_bytes;
using nosync::is_nth_bit_set;
using nosync::is_number_less;
using nosync::is_power_of_two;
using nosync::number_fits_in_type;
using std::experimental::make_array;
using std::int16_t;
using std::int32_t;
using std::int8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint8_t;


TEST(NosyncNumberUtils, CastToUnsigned) {
    ASSERT_EQ(cast_to_unsigned(static_cast<int16_t>(0xBEEFU)), 0xBEEFU);
    ASSERT_EQ(sizeof(cast_to_unsigned(static_cast<int16_t>(0xBEEFU))), sizeof(uint16_t));
    ASSERT_EQ(cast_to_unsigned(static_cast<int16_t>(-12345)), static_cast<uint16_t>(-12345));
    ASSERT_EQ(sizeof(cast_to_unsigned(static_cast<int16_t>(-12345))), sizeof(uint16_t));

    ASSERT_EQ(cast_to_unsigned(static_cast<uint16_t>(0xBEEFU)), 0xBEEFU);
    ASSERT_EQ(sizeof(cast_to_unsigned(static_cast<uint16_t>(0xBEEFU))), sizeof(uint16_t));

    ASSERT_EQ(cast_to_unsigned(static_cast<int32_t>(0xDEADBEEFU)), 0xDEADBEEFU);
    ASSERT_EQ(sizeof(cast_to_unsigned(static_cast<int32_t>(0xDEADBEEFU))), sizeof(uint32_t));
    ASSERT_EQ(cast_to_unsigned(static_cast<int32_t>(-123456789)), static_cast<uint32_t>(-123456789));
    ASSERT_EQ(sizeof(cast_to_unsigned(static_cast<int32_t>(-123456789))), sizeof(uint32_t));

    ASSERT_EQ(cast_to_unsigned(static_cast<uint32_t>(0xDEADBEEFU)), 0xDEADBEEFU);
    ASSERT_EQ(sizeof(cast_to_unsigned(static_cast<uint32_t>(0xDEADBEEFU))), sizeof(uint32_t));

    ASSERT_EQ(cast_to_unsigned(true), true);
    ASSERT_EQ(cast_to_unsigned(false), false);
}


TEST(NosyncNumberUtils, IsNThBitSet) {
    ASSERT_EQ(is_nth_bit_set(static_cast<uint8_t>(0xFF), 8), false);
    ASSERT_EQ(is_nth_bit_set(static_cast<int8_t>(0xFF), 8), false);
    ASSERT_EQ(is_nth_bit_set(static_cast<uint16_t>(0xFFFF), 16), false);
    ASSERT_EQ(is_nth_bit_set(static_cast<int16_t>(0xFFFF), 16), false);
    ASSERT_EQ(is_nth_bit_set(static_cast<uint32_t>(0xFFFFFFFF), 32), false);
    ASSERT_EQ(is_nth_bit_set(static_cast<int32_t>(0xFFFFFFFF), 32), false);

    constexpr auto value = static_cast<int32_t>(0xDEADBEEF);
    ASSERT_EQ(is_nth_bit_set(value, 0), true);
    ASSERT_EQ(is_nth_bit_set(value, 4), false);
    ASSERT_EQ(is_nth_bit_set(value, 13), true);
    ASSERT_EQ(is_nth_bit_set(value, 25), true);
    ASSERT_EQ(is_nth_bit_set(value, 31), true);
    ASSERT_EQ(is_nth_bit_set(value, 32), false);
}


TEST(NosyncNumberUtils, IsPowerOfTwo) {
    ASSERT_FALSE(is_power_of_two(static_cast<uint32_t>(0)));
    ASSERT_FALSE(is_power_of_two(static_cast<uint32_t>(0xFFFFFFFF)));

    for (unsigned i = 0; i < 32; ++i) {
        const auto bit_value = static_cast<uint32_t>(1) << i;
        ASSERT_TRUE(is_power_of_two(bit_value));
        ASSERT_TRUE(is_power_of_two(static_cast<int32_t>(bit_value)));
        ASSERT_FALSE(is_power_of_two(static_cast<uint32_t>(bit_value | 1 | 0x80000000)));
        ASSERT_FALSE(is_power_of_two(static_cast<int32_t>(bit_value | 1 | 0x80000000)));
    }
}


TEST(NosyncNumberUtils, EncodeToBeAtoms8Bit) {
    ASSERT_EQ(encode_to_be_atoms<uint8_t>(static_cast<uint8_t>(0x12)), make_array<uint8_t>(0x12));
    ASSERT_EQ(encode_to_be_atoms<uint8_t>(static_cast<uint16_t>(0x1234)), make_array<uint8_t>(0x12, 0x34));
    ASSERT_EQ(encode_to_be_atoms<uint8_t>(static_cast<uint32_t>(0x12345678)), make_array<uint8_t>(0x12, 0x34, 0x56, 0x78));
    ASSERT_EQ(encode_to_be_atoms<uint8_t>(static_cast<uint64_t>(0x123456789ABCDEF0)), make_array<uint8_t>(0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0));
}


TEST(NosyncNumberUtils, EncodeToLeAtoms8Bit) {
    ASSERT_EQ(encode_to_le_atoms<uint8_t>(static_cast<uint8_t>(0x12)), make_array<uint8_t>(0x12));
    ASSERT_EQ(encode_to_le_atoms<uint8_t>(static_cast<uint16_t>(0x1234)), make_array<uint8_t>(0x34, 0x12));
    ASSERT_EQ(encode_to_le_atoms<uint8_t>(static_cast<uint32_t>(0x12345678)), make_array<uint8_t>(0x78, 0x56, 0x34, 0x12));
    ASSERT_EQ(encode_to_le_atoms<uint8_t>(static_cast<uint64_t>(0x123456789ABCDEF0)), make_array<uint8_t>(0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12));
}


TEST(NosyncNumberUtils, EncodeToBeAtoms16Bit) {
    ASSERT_EQ(encode_to_be_atoms<uint16_t>(static_cast<uint16_t>(0x1234)), make_array<uint16_t>(0x1234));
    ASSERT_EQ(encode_to_be_atoms<uint16_t>(static_cast<uint32_t>(0x12345678)), make_array<uint16_t>(0x1234, 0x5678));
    ASSERT_EQ(encode_to_be_atoms<uint16_t>(static_cast<uint64_t>(0x123456789ABCDEF0)), make_array<uint16_t>(0x1234, 0x5678, 0x9ABC, 0xDEF0));
}


TEST(NosyncNumberUtils, EncodeToLeAtoms16Bit) {
    ASSERT_EQ(encode_to_le_atoms<uint16_t>(static_cast<uint16_t>(0x1234)), make_array<uint16_t>(0x1234));
    ASSERT_EQ(encode_to_le_atoms<uint16_t>(static_cast<uint32_t>(0x12345678)), make_array<uint16_t>(0x5678, 0x1234));
    ASSERT_EQ(encode_to_le_atoms<uint16_t>(static_cast<uint64_t>(0x123456789ABCDEF0)), make_array<uint16_t>(0xDEF0, 0x9ABC, 0x5678, 0x1234));
}


TEST(NosyncNumberUtils, EncodeToBeBytes) {
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint8_t>(0xF1)), make_array<char>('\xF1'));
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint16_t>(0xF1)), make_array<char>('\x00', '\xF1'));
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint32_t>(0xF1)), make_array<char>('\x00', '\x00', '\x00', '\xF1'));
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint64_t>(0xF1)), make_array<char>('\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xF1'));

    ASSERT_EQ(encode_to_be_bytes(static_cast<uint8_t>(0x12)), make_array<char>('\x12'));
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint16_t>(0x1234)), make_array<char>('\x12', '\x34'));
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint32_t>(0x12345678)), make_array<char>('\x12', '\x34', '\x56', '\x78'));
    ASSERT_EQ(encode_to_be_bytes(static_cast<uint64_t>(0x123456789ABCDEF0)), make_array<char>('\x12', '\x34', '\x56', '\x78', '\x9A', '\xBC', '\xDE', '\xF0'));
}


TEST(NosyncNumberUtils, EncodeToLeBytes) {
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint8_t>(0xF1)), make_array<char>('\xF1'));
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint16_t>(0xF1)), make_array<char>('\xF1', '\x00'));
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint32_t>(0xF1)), make_array<char>('\xF1', '\x00', '\x00', '\x00'));
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint64_t>(0xF1)), make_array<char>('\xF1', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'));

    ASSERT_EQ(encode_to_le_bytes(static_cast<uint8_t>(0x12)), make_array<char>('\x12'));
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint16_t>(0x1234)), make_array<char>('\x34', '\x12'));
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint32_t>(0x12345678)), make_array<char>('\x78', '\x56', '\x34', '\x12'));
    ASSERT_EQ(encode_to_le_bytes(static_cast<uint64_t>(0x123456789ABCDEF0)), make_array<char>('\xF0', '\xDE', '\xBC', '\x9A', '\x78', '\x56', '\x34', '\x12'));
}


TEST(NosyncNumberUtils, DecodeBeAtoms8BitToNumber) {
    ASSERT_EQ((decode_be_atoms_to_number<uint8_t, uint8_t>(make_array<uint8_t>(0xFB))), 0xFBU);
    ASSERT_EQ((decode_be_atoms_to_number<uint8_t, uint16_t>(make_array<uint8_t>(0x39, 0xFB))), 0x39FBU);
    ASSERT_EQ((decode_be_atoms_to_number<uint8_t, uint32_t>(make_array<uint8_t>(0xBA, 0x49, 0x39, 0xFB))), 0xBA4939FBU);
}


TEST(NosyncNumberUtils, DecodeBeAtoms16BitToNumber) {
    ASSERT_EQ((decode_be_atoms_to_number<uint16_t, uint16_t>(make_array<uint16_t>(0x39FB))), 0x39FBU);
    ASSERT_EQ((decode_be_atoms_to_number<uint16_t, uint32_t>(make_array<uint16_t>(0xBA49, 0x39FB))), 0xBA4939FBU);
}


TEST(NosyncNumberUtils, DecodeLeAtoms8BitToNumber) {
    ASSERT_EQ((decode_le_atoms_to_number<uint8_t, uint8_t>(make_array<uint8_t>(0xFB))), 0xFBU);
    ASSERT_EQ((decode_le_atoms_to_number<uint8_t, uint16_t>(make_array<uint8_t>(0x39, 0xFB))), 0xFB39U);
    ASSERT_EQ((decode_le_atoms_to_number<uint8_t, uint32_t>(make_array<uint8_t>(0xBA, 0x49, 0x39, 0xFB))), 0xFB3949BAU);
}


TEST(NosyncNumberUtils, DecodeLeAtoms16BitToNumber) {
    ASSERT_EQ((decode_le_atoms_to_number<uint16_t, uint16_t>(make_array<uint16_t>(0x39FB))), 0x39FBU);
    ASSERT_EQ((decode_le_atoms_to_number<uint16_t, uint32_t>(make_array<uint16_t>(0xBA49, 0x39FB))), 0x39FBBA49U);
}


TEST(NosyncNumberUtils, DecodeBeBytesToNumberFromArray) {
    ASSERT_EQ(decode_be_bytes_to_number<uint8_t>(make_array('\xFB')), 0xFBU);
    ASSERT_EQ(decode_be_bytes_to_number<int8_t>(make_array('\xFB')), static_cast<int8_t>(0xFBU));

    ASSERT_EQ(decode_be_bytes_to_number<uint16_t>(make_array('\x39', '\xFB')), 0x39FBU);
    ASSERT_EQ(decode_be_bytes_to_number<uint16_t>(make_array('\xFB')), 0xFBU);
    ASSERT_EQ(decode_be_bytes_to_number<int16_t>(make_array('\x39', '\xFB')), static_cast<int16_t>(0x39FBU));
    ASSERT_EQ(decode_be_bytes_to_number<int16_t>(make_array('\xFB')), static_cast<int16_t>(0xFBU));

    ASSERT_EQ(decode_be_bytes_to_number<uint32_t>(make_array('\xBA', '\x49', '\x39', '\xFB')), 0xBA4939FBU);
    ASSERT_EQ(decode_be_bytes_to_number<uint32_t>(make_array('\x39', '\xFB')), 0x39FBU);
    ASSERT_EQ(decode_be_bytes_to_number<int32_t>(make_array('\xBA', '\x49', '\x39', '\xFB')), static_cast<int32_t>(0xBA4939FBU));
    ASSERT_EQ(decode_be_bytes_to_number<int32_t>(make_array('\x39', '\xFB')), static_cast<int32_t>(0x39FBU));
}


TEST(NosyncNumberUtils, DecodeLeBytesToNumberFromArray) {
    ASSERT_EQ(decode_le_bytes_to_number<uint8_t>(make_array('\xFB')), 0xFBU);
    ASSERT_EQ(decode_le_bytes_to_number<int8_t>(make_array('\xFB')), static_cast<int8_t>(0xFBU));

    ASSERT_EQ(decode_le_bytes_to_number<uint16_t>(make_array('\x39', '\xFB')), 0xFB39U);
    ASSERT_EQ(decode_le_bytes_to_number<uint16_t>(make_array('\xFB')), 0xFBU);
    ASSERT_EQ(decode_le_bytes_to_number<int16_t>(make_array('\x39', '\xFB')), static_cast<int16_t>(0xFB39U));
    ASSERT_EQ(decode_le_bytes_to_number<int16_t>(make_array('\xFB')), static_cast<int16_t>(0xFBU));

    ASSERT_EQ(decode_le_bytes_to_number<uint32_t>(make_array('\xBA', '\x49', '\x39', '\xFB')), 0xFB3949BAU);
    ASSERT_EQ(decode_le_bytes_to_number<uint32_t>(make_array('\x39', '\xFB')), 0xFB39U);
    ASSERT_EQ(decode_le_bytes_to_number<int32_t>(make_array('\xBA', '\x49', '\x39', '\xFB')), static_cast<int32_t>(0xFB3949BAU));
    ASSERT_EQ(decode_le_bytes_to_number<int32_t>(make_array('\x39', '\xFB')), static_cast<int32_t>(0xFB39U));
}


TEST(NosyncNumberUtils, DecodeBeBytesToNumberFromStringView) {
    ASSERT_EQ(decode_be_bytes_to_number<uint8_t>("\xFB"s), 0xFBU);
    ASSERT_EQ(decode_be_bytes_to_number<int8_t>("\xFB"s), static_cast<int8_t>(0xFBU));

    ASSERT_EQ(decode_be_bytes_to_number<uint16_t>("\x39\xFB"s), 0x39FBU);
    ASSERT_EQ(decode_be_bytes_to_number<uint16_t>("\xFB"s), 0xFBU);
    ASSERT_EQ(decode_be_bytes_to_number<int16_t>("\x39\xFB"s), static_cast<int16_t>(0x39FBU));
    ASSERT_EQ(decode_be_bytes_to_number<int16_t>("\xFB"s), static_cast<int16_t>(0xFBU));

    ASSERT_EQ(decode_be_bytes_to_number<uint32_t>("\xBA\x49\x39\xFB"s), 0xBA4939FBU);
    ASSERT_EQ(decode_be_bytes_to_number<uint32_t>("\x39\xFB"s), 0x39FBU);
    ASSERT_EQ(decode_be_bytes_to_number<int32_t>("\xBA\x49\x39\xFB"s), static_cast<int32_t>(0xBA4939FBU));
    ASSERT_EQ(decode_be_bytes_to_number<int32_t>("\x39\xFB"s), static_cast<int32_t>(0x39FBU));
}


TEST(NosyncNumberUtils, DecodeBeBytesToNumberFromStringViewInputEmpty) {
    ASSERT_EQ(decode_be_bytes_to_number<uint8_t>(""s), 0);
    ASSERT_EQ(decode_be_bytes_to_number<uint16_t>(""s), 0);
}


TEST(NosyncNumberUtils, DecodeBeBytesToNumberFromStringViewInputTooBig) {
    ASSERT_THROW(decode_be_bytes_to_number<uint16_t>("\x39\xFB\x12"s), std::invalid_argument);
}


TEST(NosyncNumberUtils, DecodeLeBytesToNumberFromStringView) {
    ASSERT_EQ(decode_le_bytes_to_number<uint8_t>("\xFB"s), 0xFBU);
    ASSERT_EQ(decode_le_bytes_to_number<int8_t>("\xFB"s), static_cast<int8_t>(0xFBU));

    ASSERT_EQ(decode_le_bytes_to_number<uint16_t>("\x39\xFB"s), 0xFB39U);
    ASSERT_EQ(decode_le_bytes_to_number<uint16_t>("\xFB"s), 0xFBU);
    ASSERT_EQ(decode_le_bytes_to_number<int16_t>("\x39\xFB"s), static_cast<int16_t>(0xFB39U));
    ASSERT_EQ(decode_le_bytes_to_number<int16_t>("\xFB"s), static_cast<int16_t>(0xFBU));

    ASSERT_EQ(decode_le_bytes_to_number<uint32_t>("\xBA\x49\x39\xFB"s), 0xFB3949BAU);
    ASSERT_EQ(decode_le_bytes_to_number<uint32_t>("\x39\xFB"s), 0xFB39U);
    ASSERT_EQ(decode_le_bytes_to_number<int32_t>("\xBA\x49\x39\xFB"s), static_cast<int32_t>(0xFB3949BAU));
    ASSERT_EQ(decode_le_bytes_to_number<int32_t>("\x39\xFB"s), static_cast<int32_t>(0xFB39U));
}


TEST(NosyncNumberUtils, DecodeLeBytesToNumberFromStringViewInputEmpty) {
    ASSERT_EQ(decode_le_bytes_to_number<uint8_t>(""s), 0);
    ASSERT_EQ(decode_le_bytes_to_number<uint16_t>(""s), 0);
}


TEST(NosyncNumberUtils, DecodeLeBytesToNumberFromStringViewInputTooBig) {
    ASSERT_THROW(decode_le_bytes_to_number<uint16_t>("\x39\xFB\x12"s), std::invalid_argument);
}


TEST(NosyncNumberUtils, IsNumberLess) {
    ASSERT_TRUE(is_number_less(-4, 20));
    ASSERT_FALSE(is_number_less(100, 100));
    ASSERT_TRUE(is_number_less(static_cast<int8_t>(-128), static_cast<uint8_t>(127)));
    ASSERT_TRUE(is_number_less(static_cast<int8_t>(-127), static_cast<uint8_t>(127)));
    ASSERT_TRUE(is_number_less(static_cast<int8_t>(-1), static_cast<uint8_t>(127)));
    ASSERT_TRUE(is_number_less(static_cast<int16_t>(-32768), static_cast<uint16_t>(32767)));
    ASSERT_TRUE(is_number_less(static_cast<int16_t>(-32767), static_cast<uint16_t>(32767)));
    ASSERT_TRUE(is_number_less(static_cast<int16_t>(-1), static_cast<uint16_t>(32767)));
    ASSERT_TRUE(is_number_less(static_cast<int32_t>(-2147483647), static_cast<uint32_t>(2147483647)));
    ASSERT_TRUE(is_number_less(static_cast<int32_t>(-2147483648), static_cast<uint32_t>(2147483647)));
    ASSERT_TRUE(is_number_less(static_cast<int32_t>(-1), static_cast<uint32_t>(2147483647)));
    ASSERT_TRUE(is_number_less(static_cast<int8_t>(-1), static_cast<uint32_t>(0)));
    ASSERT_TRUE(is_number_less(static_cast<int16_t>(-1), static_cast<uint32_t>(0)));
}


TEST(NosyncNumberUtils, NumberFitsInType) {
    ASSERT_TRUE(number_fits_in_type<int8_t>(12L));
    ASSERT_TRUE(number_fits_in_type<int8_t>(-50L));
    ASSERT_TRUE(number_fits_in_type<int8_t>(-128L));
    ASSERT_TRUE(number_fits_in_type<int8_t>(127L));
    ASSERT_FALSE(number_fits_in_type<int8_t>(128L));

    ASSERT_TRUE(number_fits_in_type<uint8_t>(12L));
    ASSERT_FALSE(number_fits_in_type<uint8_t>(-50L));
    ASSERT_FALSE(number_fits_in_type<uint8_t>(-128L));
    ASSERT_TRUE(number_fits_in_type<uint8_t>(127L));
    ASSERT_TRUE(number_fits_in_type<uint8_t>(128L));
    ASSERT_TRUE(number_fits_in_type<uint8_t>(255L));
    ASSERT_FALSE(number_fits_in_type<uint8_t>(256L));

    ASSERT_TRUE(number_fits_in_type<int16_t>(767L));
    ASSERT_TRUE(number_fits_in_type<int16_t>(-519L));
    ASSERT_TRUE(number_fits_in_type<int16_t>(-32768L));
    ASSERT_TRUE(number_fits_in_type<int16_t>(32767L));
    ASSERT_FALSE(number_fits_in_type<int16_t>(32768L));

    ASSERT_TRUE(number_fits_in_type<uint16_t>(767L));
    ASSERT_FALSE(number_fits_in_type<uint16_t>(-519L));
    ASSERT_FALSE(number_fits_in_type<uint16_t>(-32768L));
    ASSERT_TRUE(number_fits_in_type<uint16_t>(32767L));
    ASSERT_TRUE(number_fits_in_type<uint16_t>(32768L));
    ASSERT_TRUE(number_fits_in_type<uint16_t>(65535L));
    ASSERT_FALSE(number_fits_in_type<uint16_t>(65536L));

    ASSERT_TRUE(number_fits_in_type<uint32_t>(0x12L));
    ASSERT_TRUE(number_fits_in_type<uint32_t>(0xFFFFFFFFL));
    ASSERT_FALSE(number_fits_in_type<uint32_t>(0x100000000L));
}
