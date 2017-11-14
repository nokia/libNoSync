// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TYPE_UTILS_H
#define NOSYNC__TYPE_UTILS_H

#include <cstddef>


namespace nosync
{

template<typename T>
constexpr std::size_t get_sizeof_in_bits() noexcept;

template<typename Atom, typename T>
constexpr std::size_t get_sizeof_in_atoms() noexcept;

template<typename ...T>
constexpr std::size_t get_sizeof_sum() noexcept;

template<typename T>
constexpr std::size_t sizeof_in_bits = get_sizeof_in_bits<T>();

template<typename Atom, typename T>
constexpr std::size_t sizeof_in_atoms = get_sizeof_in_atoms<Atom, T>();

template<typename ...T>
constexpr std::size_t sizeof_sum = get_sizeof_sum<T...>();

template<typename T>
T make_copy(const T &value);

}

#include <nosync/type-utils-impl.h>

#endif /* NOSYNC__TYPE_UTILS_H */
