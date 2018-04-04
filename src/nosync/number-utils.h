// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__NUMBER_UTILS_H
#define NOSYNC__NUMBER_UTILS_H

#include <array>
#include <cstdint>
#include <experimental/string_view>
#include <nosync/type-utils.h>


namespace nosync
{

template<typename T>
constexpr auto cast_to_unsigned(T value) noexcept;

template<typename T>
constexpr bool is_nth_bit_set(T value, unsigned n) noexcept;

template<typename T>
constexpr bool is_power_of_two(T value) noexcept;

template<typename Atom, typename T>
constexpr std::array<Atom, sizeof_in_atoms<Atom, T>> encode_to_be_atoms(T value) noexcept;

template<typename Atom, typename T>
constexpr std::array<Atom, sizeof_in_atoms<Atom, T>> encode_to_le_atoms(T value) noexcept;

template<typename Atom, typename T, std::size_t N>
constexpr T decode_be_atoms_to_number(const std::array<Atom, sizeof_in_atoms<Atom, T>> &atoms) noexcept;

template<typename Atom, typename T, std::size_t N>
constexpr T decode_le_atoms_to_number(const std::array<Atom, sizeof_in_atoms<Atom, T>> &atoms) noexcept;

template<typename T>
constexpr std::array<char, sizeof(T)> encode_to_be_bytes(T value) noexcept;

template<typename T>
constexpr std::array<char, sizeof(T)> encode_to_le_bytes(T value) noexcept;

template<typename T, std::size_t N>
constexpr T decode_be_bytes_to_number(const std::array<char, sizeof(T)> &bytes) noexcept;

template<typename T, std::size_t N>
constexpr T decode_le_bytes_to_number(const std::array<char, sizeof(T)> &bytes) noexcept;

template<typename T = std::uintmax_t>
constexpr T decode_be_bytes_to_number(std::experimental::string_view bytes);

template<typename T = std::uintmax_t>
constexpr T decode_le_bytes_to_number(std::experimental::string_view bytes);

template<typename TL, typename TR>
constexpr bool is_number_less(TL lhs, TR rhs) noexcept;

template<typename T, typename TV>
constexpr bool number_fits_in_type(TV number) noexcept;

}

#include <nosync/number-utils-impl.h>

#endif /* NOSYNC__NUMBER_UTILS_H */
