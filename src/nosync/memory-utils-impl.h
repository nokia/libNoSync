// This file is part of libnosync library. See LICENSE file for license details.
#include <type_traits>
#include <utility>


namespace nosync
{

template<typename T>
std::weak_ptr<T> weak_from_shared(std::shared_ptr<T> ptr)
{
    return std::move(ptr);
}


template<typename T>
std::weak_ptr<T> weak_from_that(T *obj)
{
    static_assert(
        std::is_base_of<std::enable_shared_from_this<T>, T>::value,
        "type must inherit from enable_shared_from_this<>");
    return obj->shared_from_this();
}


template<typename T, typename F>
auto make_weak_this_func_proxy(T *self, F func)
{
    return [self_wptr = weak_from_that(self), func = std::move(func)]() {
        auto self_ptr = self_wptr.lock();
        if (self_ptr) {
            func(*self_ptr);
        }
    };
}

}
