// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__CONTAINER_UTILS_IMPL_H
#define NOSYNC__CONTAINER_UTILS_IMPL_H

#include <experimental/array>
#include <utility>


namespace nosync

{

namespace container_utils_impl
{

template<typename T, std::size_t N, typename F, std::size_t ...I>
constexpr std::array<T, N> generate_array(const F &elem_generator, std::index_sequence<I...>)
{
    return std::experimental::make_array(elem_generator(I)...);
}

};


template<typename T, std::size_t N, typename F>
constexpr std::array<T, N> generate_array(const F &elem_generator)
{
    return container_utils_impl::generate_array<T, N, F>(elem_generator, std::make_index_sequence<N>());
}

}

#endif /* NOSYNC__CONTAINER_UTILS_IMPL_H */
