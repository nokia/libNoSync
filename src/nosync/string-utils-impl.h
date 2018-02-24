// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__STRING_UTILS_IMPL_H
#define NOSYNC__STRING_UTILS_IMPL_H

#include <array>
#include <cstddef>
#include <experimental/array>
#include <nosync/type-utils.h>
#include <type_traits>
#include <utility>


namespace nosync
{

namespace string_utils_impl
{

constexpr char to_hex_digit(unsigned value) noexcept
{
    const auto eff_value = value & 0x0F;
    char digit = eff_value < 10 ? '0' + eff_value : 'A' + eff_value - 10;
    return digit;
}


template<typename T, std::size_t... I>
constexpr std::array<char, sizeof(T) * 2> number_to_hex_digits_array_impl(T value, std::index_sequence<I...>) noexcept
{
    constexpr auto nibbles_count = sizeof...(I);
    constexpr auto nibble_size_in_bits = 4;

    const auto unsigned_value = static_cast<std::make_unsigned_t<T>>(value);

    return std::experimental::make_array(
        to_hex_digit(unsigned_value >> ((nibbles_count - 1 - I) * nibble_size_in_bits))...);
}

}


template<typename T>
constexpr std::array<char, sizeof(T) * 2> number_to_hex_digits_array(T value) noexcept
{
    return string_utils_impl::number_to_hex_digits_array_impl(value, std::make_index_sequence<sizeof(T) * 2>());
}


template<std::size_t N>
std::string make_string(const std::array<char, N> &bytes)
{
    return {bytes.data(), bytes.size()};
}


template<std::size_t N>
constexpr std::experimental::string_view make_string_view(const std::array<char, N> &bytes) noexcept
{
    return {bytes.data(), bytes.size()};
}

}

#endif /* NOSYNC__STRING_UTILS_IMPL_H */
