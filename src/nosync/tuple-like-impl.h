// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TUPLE_LIKE_IMPL_H
#define NOSYNC__TUPLE_LIKE_IMPL_H

#include <functional>
#include <tuple>
#include <utility>


namespace nosync
{

namespace tuple_like_impl
{

template<typename T, typename EE, size_t... I>
constexpr auto get_tuple_like_class_members_impl(const T &value, const EE &value_cls_fields, std::index_sequence<I...>)
{
    return std::make_tuple(std::cref((&value)->*std::get<I>(value_cls_fields))...);
}


template<typename T, typename EE, size_t... I>
constexpr auto get_tuple_like_class_members_impl(T &value, const EE &value_cls_fields, std::index_sequence<I...>)
{
    return std::make_tuple(std::ref((&value)->*std::get<I>(value_cls_fields))...);
}

}


template<typename T, typename>
struct is_tuple_like : public std::false_type
{
};


template<typename T>
struct is_tuple_like<T, std::experimental::void_t<decltype(std::is_base_of<tuple_like<T>, T>::value)>> : public std::is_base_of<tuple_like<T>, T>
{
};


template<typename T>
constexpr auto get_tuple_like_class_members(const T &value)
{
    constexpr auto cls_fields = T::cls_fields;
    return tuple_like_impl::get_tuple_like_class_members_impl(
        value, cls_fields, std::make_index_sequence<std::tuple_size<decltype(cls_fields)>::value>());
}

}

#endif /* NOSYNC__TUPLE_LIKE_IMPL_H */
