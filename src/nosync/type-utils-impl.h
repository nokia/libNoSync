// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TYPE_UTILS_IMPL_H
#define NOSYNC__TYPE_UTILS_IMPL_H

#include <climits>
#include <limits>


namespace nosync
{

namespace type_utils_impl
{

template<typename T1 = void>
constexpr std::enable_if_t<std::is_void<T1>::value, std::size_t> get_sizeof_sum_impl() noexcept
{
    return 0;
}


template<typename T1, typename ...TT>
constexpr std::enable_if_t<!std::is_void<T1>::value, std::size_t> get_sizeof_sum_impl() noexcept
{
    return sizeof(T1) + get_sizeof_sum<TT...>();
}

}


template<typename T>
constexpr std::size_t get_sizeof_in_bits() noexcept
{
    static_assert(sizeof(T) <= std::numeric_limits<size_t>::max() / CHAR_BIT, "size in bits must fit in integer type");

    return static_cast<std::size_t>(sizeof(T) * CHAR_BIT);
}


template<typename Atom, typename T>
constexpr std::size_t get_sizeof_in_atoms() noexcept
{
    static_assert(sizeof(T) % sizeof(Atom) == 0, "type size must be integer multiple of atom size");

    return sizeof(T) / sizeof(Atom);
}


template<typename ...T>
constexpr std::size_t get_sizeof_sum() noexcept
{
    return type_utils_impl::get_sizeof_sum_impl<T...>();
}


template<typename T>
T make_copy(const T &value)
{
    return value;
}

}

#endif /* NOSYNC__TYPE_UTILS_IMPL_H */
