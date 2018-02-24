// This file is part of libnosync library. See LICENSE file for license details.
#include <cstring>
#include <locale>
#include <nosync/string-utils.h>
#include <stdexcept>
#include <string>


namespace nosync
{

using std::experimental::string_view;
using std::invalid_argument;
using std::isprint;
using std::locale;
using std::size_t;
using std::string;
using std::to_string;


namespace
{

constexpr char number_to_hex_digit(unsigned value) noexcept
{
    const auto eff_value = value & 0x0F;
    char digit = eff_value < 10 ? '0' + eff_value : 'A' + eff_value - 10;
    return digit;
}


unsigned number_from_hex_digit(char hex_digit)
{
    unsigned digit_value;
    if (hex_digit >= '0' && hex_digit <= '9') {
        digit_value = hex_digit - '0';
    } else if (hex_digit >= 'A' && hex_digit <= 'F') {
        digit_value = hex_digit - 'A' + 10;
    } else if (hex_digit >= 'a' && hex_digit <= 'f') {
        digit_value = hex_digit - 'a' + 10;
    } else {
        throw invalid_argument("illegal hex digit: " + string(1, hex_digit));
    }

    return digit_value;
}


string to_hex_escaped_printable_string_impl(string_view input)
{
    constexpr auto escape_char = '\\';
    const auto &c_locale = locale::classic();

    size_t output_size = 0;
    for (const auto ch : input) {
        if (ch == escape_char) {
            output_size += 2;
        } else if (isprint(ch, c_locale)) {
            ++output_size;
        } else {
            output_size += 4;
        }
    }

    string output;
    output.reserve(output_size);

    for (const auto ch : input) {
        if (ch == escape_char) {
            output.push_back(escape_char);
            output.push_back(ch);
        } else if (isprint(ch, c_locale)) {
            output.push_back(ch);
        } else {
            output.push_back(escape_char);
            output.push_back('x');
            auto hex_digits = number_to_hex_digits_array(ch);
            output.append(hex_digits.cbegin(), hex_digits.cend());
        }
    }

    return output;
}

}


string bytes_to_hex_string(string_view bytes)
{
    string hex_str;
    hex_str.reserve(bytes.size() * 2);

    for (auto byte : bytes) {
        auto byte_value = static_cast<unsigned char>(byte);
        hex_str += number_to_hex_digit(byte_value >> 4);
        hex_str += number_to_hex_digit(byte_value & 0x0F);
    }

    return hex_str;
}


string bytes_from_hex_string(string_view hex_string)
{
    if (hex_string.size() % 2 != 0) {
        throw invalid_argument("odd length of hex string: " + to_string(hex_string.size()));
    }

    string bytes;
    bytes.reserve(hex_string.size() / 2);

    for (size_t i = 0; i < hex_string.size(); i += 2) {
        const auto hi_nibble = number_from_hex_digit(hex_string[i]);
        const auto lo_nibble = number_from_hex_digit(hex_string[i + 1]);
        bytes.push_back((hi_nibble << 4) | lo_nibble);
    }

    return bytes;
}


string to_hex_escaped_printable_string(string_view input)
{
    return to_hex_escaped_printable_string_impl(input);
}

}
