// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__STRING_UTILS_H
#define NOSYNC__STRING_UTILS_H

#include <array>
#include <experimental/string_view>
#include <string>


namespace nosync
{

std::string bytes_to_hex_string(std::experimental::string_view bytes);
std::string bytes_from_hex_string(std::experimental::string_view hex_string);

template<typename T>
constexpr std::array<char, sizeof(T) * 2> number_to_hex_digits_array(T value) noexcept;

template<std::size_t N>
std::string make_string(const std::array<char, N> &bytes);

template<std::size_t N>
constexpr std::experimental::string_view make_string_view(const std::array<char, N> &bytes) noexcept;

std::string to_hex_escaped_printable_string(std::experimental::string_view input);

}

#include <nosync/string-utils-impl.h>

#endif /* NOSYNC__STRING_UTILS_H */
