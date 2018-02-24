// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__NUMBER_UTILS_IMPL_H
#define NOSYNC__NUMBER_UTILS_IMPL_H

#include <experimental/array>
#include <experimental/type_traits>
#include <nosync/type-utils.h>
#include <utility>
#include <stdexcept>
#include <string>


namespace nosync
{

namespace number_utils_impl
{

using std::experimental::make_array;
using std::size_t;


template<typename Atom, typename T>
constexpr auto get_nth_atom(T value, size_t n) noexcept
{
    const auto unsigned_value = cast_to_unsigned(value);
    return static_cast<Atom>(unsigned_value >> (n * sizeof_in_bits<Atom>));
}


template<typename Atom, typename T, size_t... I>
constexpr auto encode_to_be_atoms_impl(T value, std::index_sequence<I...>) noexcept
{
    return make_array(get_nth_atom<Atom>(value, sizeof_in_atoms<Atom, T> - 1 - I)...);
}


template<typename Atom, typename T, size_t... I>
constexpr auto encode_to_le_atoms_impl(T value, std::index_sequence<I...>) noexcept
{
    return make_array(get_nth_atom<Atom>(value, I)...);
}


template<typename TL, typename TR>
constexpr std::enable_if_t<std::is_signed<TL>::value && !std::is_signed<TR>::value, bool>
is_number_less(TL lhs, TR rhs) noexcept
{
    return lhs < 0 || static_cast<std::make_unsigned_t<TL>>(lhs) < rhs;
}


template<typename TL, typename TR>
constexpr std::enable_if_t<!std::is_signed<TL>::value && std::is_signed<TR>::value, bool>
is_number_less(TL lhs, TR rhs) noexcept
{
    return rhs >= 0 && lhs < static_cast<std::make_unsigned_t<TR>>(rhs);
}


template<typename TL, typename TR>
constexpr std::enable_if_t<std::is_signed<TL>::value == std::is_signed<TR>::value, bool>
is_number_less(TL lhs, TR rhs) noexcept
{
    return lhs < rhs;
}

}


template<typename T>
constexpr auto cast_to_unsigned(T value) noexcept
{
    static_assert(std::experimental::is_integral_v<T>, "value must have integral type");

    return static_cast<std::make_unsigned_t<T>>(value);
}


template<>
constexpr auto cast_to_unsigned(bool value) noexcept
{
    return value;
}


template<typename T>
constexpr bool is_nth_bit_set(T value, unsigned n) noexcept
{
    static_assert(std::experimental::is_integral_v<T>, "value must have integer type");

    return n < sizeof_in_bits<T> && (value & (static_cast<T>(1) << n)) != 0;
}


template<typename T>
constexpr bool is_power_of_two(T value) noexcept
{
    static_assert(std::experimental::is_integral_v<T>, "only integer types are supported");

    const auto unsigned_value = cast_to_unsigned(value);

    return (unsigned_value & (unsigned_value - 1)) == 0 && unsigned_value != 0;
}


template<typename Atom, typename T>
constexpr std::array<Atom, sizeof_in_atoms<Atom, T>> encode_to_be_atoms(T value) noexcept
{
    static_assert(std::experimental::is_integral_v<T>, "only integer types are supported");

    return number_utils_impl::encode_to_be_atoms_impl<Atom>(
        value, std::make_index_sequence<sizeof_in_atoms<Atom, T>>());
}


template<typename Atom, typename T>
constexpr std::array<Atom, sizeof_in_atoms<Atom, T>> encode_to_le_atoms(T value) noexcept
{
    static_assert(std::experimental::is_integral_v<T>, "only integer types are supported");

    return number_utils_impl::encode_to_le_atoms_impl<Atom>(
        value, std::make_index_sequence<sizeof_in_atoms<Atom, T>>());
}


template<typename Atom, typename T, std::size_t N>
constexpr T decode_be_atoms_to_number(const std::array<Atom, N> &atoms) noexcept
{
    static_assert(N <= sizeof_in_atoms<Atom, T>, "size of input array must not exceed the size of target type");

    T value = 0;
    for (size_t i = 0; i < atoms.size(); ++i) {
        value |= static_cast<T>(cast_to_unsigned(atoms[i])) << (sizeof_in_bits<Atom> * (atoms.size() - 1 - i));
    }

    return value;
}


template<typename Atom, typename T, std::size_t N>
constexpr T decode_le_atoms_to_number(const std::array<Atom, N> &atoms) noexcept
{
    static_assert(N <= sizeof_in_atoms<Atom, T>, "size of input array must not exceed the size of target type");

    T value = 0;
    for (size_t i = 0; i < atoms.size(); ++i) {
        value |= static_cast<T>(cast_to_unsigned(atoms[i])) << (sizeof_in_bits<Atom> * i);
    }

    return value;
}


template<typename T>
constexpr std::array<char, sizeof(T)> encode_to_be_bytes(T value) noexcept
{
    return encode_to_be_atoms<char>(value);
}


template<typename T>
constexpr std::array<char, sizeof(T)> encode_to_le_bytes(T value) noexcept
{
    return encode_to_le_atoms<char>(value);
}


template<typename T, std::size_t N>
constexpr T decode_be_bytes_to_number(const std::array<char, N> &bytes) noexcept
{
    return decode_be_atoms_to_number<char, T>(bytes);
}


template<typename T, std::size_t N>
constexpr T decode_le_bytes_to_number(const std::array<char, N> &bytes) noexcept
{
    return decode_le_atoms_to_number<char, T>(bytes);
}


template<typename T>
constexpr T decode_be_bytes_to_number(std::experimental::string_view bytes)
{
    using namespace std::string_literals;
    if (bytes.size() > sizeof(T)) {
        throw std::invalid_argument("too many input bytes for target type: "s + std::to_string(bytes.size()));
    }

    T value = 0;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        value |= static_cast<T>(cast_to_unsigned(bytes[i])) << (sizeof_in_bits<char> * (bytes.size() - 1 - i));
    }

    return value;
}


template<typename T>
constexpr T decode_le_bytes_to_number(std::experimental::string_view bytes)
{
    using namespace std::string_literals;
    if (bytes.size() > sizeof(T)) {
        throw std::invalid_argument("too many input bytes for target type: "s + std::to_string(bytes.size()));
    }

    T value = 0;
    for (size_t i = 0; i < bytes.size(); ++i) {
        value |= static_cast<T>(cast_to_unsigned(bytes[i])) << (sizeof_in_bits<char> * i);
    }

    return value;
}


template<typename TL, typename TR>
constexpr bool is_number_less(TL lhs, TR rhs) noexcept
{
    static_assert(std::experimental::is_integral_v<TL> && std::experimental::is_integral_v<TR>, "need integral parameters");
    return number_utils_impl::is_number_less(lhs, rhs);
}


template<typename T, typename TV>
constexpr bool number_fits_in_type(TV number) noexcept
{
    static_assert(std::experimental::is_integral_v<T> && std::experimental::is_integral_v<TV>, "need integral parameters");
    constexpr auto min_for_type = std::numeric_limits<T>::min();
    constexpr auto max_for_type = std::numeric_limits<T>::max();
    return !(is_number_less(number, min_for_type) || is_number_less(max_for_type, number));
}

}

#endif /* NOSYNC__NUMBER_UTILS_IMPL_H */
