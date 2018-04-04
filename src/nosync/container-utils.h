// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__CONTAINER_UTILS_H
#define NOSYNC__CONTAINER_UTILS_H

#include <array>
#include <cstddef>


namespace nosync
{

template<typename T, std::size_t N, typename F>
constexpr std::array<T, N> generate_array(const F &elem_generator);

}

#include <nosync/container-utils-impl.h>

#endif /* NOSYNC__CONTAINER_UTILS_H */
