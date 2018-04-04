// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__MEMORY_UTILS_H
#define NOSYNC__MEMORY_UTILS_H

#include <memory>


namespace nosync
{

template<typename T>
std::weak_ptr<T> weak_from_shared(std::shared_ptr<T> ptr);

template<typename T>
std::weak_ptr<T> weak_from_that(T *obj);

template<typename T, typename F>
auto make_weak_this_func_proxy(T *self, F func);

}

#include <nosync/memory-utils-impl.h>

#endif /* NOSYNC__MEMORY_UTILS_H */
