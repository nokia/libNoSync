// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TUPLE_LIKE_H
#define NOSYNC__TUPLE_LIKE_H

#include <experimental/type_traits>
#include <type_traits>


namespace nosync
{

template<typename T>
class tuple_like
{
public:
    using cls = T;
};


template<typename T, typename = void>
struct is_tuple_like;

template<typename T>
struct is_tuple_like<T, std::experimental::void_t<decltype(std::is_base_of<tuple_like<T>, T>::value)>>;


template<typename T>
constexpr bool is_tuple_like_v = is_tuple_like<T>::value;


template<typename T>
constexpr auto get_tuple_like_class_members(const T &value);

}

#include <nosync/tuple-like-impl.h>

#endif /* NOSYNC__TUPLE_LIKE_H */
