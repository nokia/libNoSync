// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_IMPL_H
#define NOSYNC__RESULT_IMPL_H

#include <stdexcept>
#include <utility>


namespace nosync
{

template<typename T>
result<T> result<T>::create_with_value(const T &value)
{
    return {{value, std::error_code()}};
}


template<typename T>
result<T> result<T>::create_with_value(T &&value)
{
    return {{std::move(value), std::error_code()}};
}


template<typename T>
result<T> result<T>::create_with_error(std::error_code ec)
{
    return {{std::experimental::nullopt, ec}};
}


template<typename T>
bool result<T>::is_ok() const
{
    return std::get<std::experimental::optional<T>>(res_err) != std::experimental::nullopt;
}


template<typename T>
T &result<T>::get_value() &
{
    check_is_ok();
    return std::get<std::experimental::optional<T>>(res_err).value();
}


template<typename T>
const T &result<T>::get_value() const &
{
    check_is_ok();
    return std::get<std::experimental::optional<T>>(res_err).value();
}


template<typename T>
T &&result<T>::get_value() &&
{
    check_is_ok();
    return std::move(std::get<std::experimental::optional<T>>(res_err).value());
}


template<typename T>
std::error_code result<T>::get_error() const
{
    if (is_ok()) {
        throw std::logic_error("reading error from async result which contains value");
    }

    return std::get<std::error_code>(res_err);
}


template<typename T>
template<typename U>
result<T>::operator result<U>() const &
{
    return is_ok()
        ? result<U>::create_with_value(get_value())
        : result<U>::create_with_error(get_error());
}


template<typename T>
template<typename U>
result<T>::operator result<U>() &&
{
    return is_ok()
        ? result<U>::create_with_value(std::move(get_value()))
        : result<U>::create_with_error(get_error());
}


template<typename T>
result<T>::result(std::tuple<std::experimental::optional<T>, std::error_code> res_err)
    : res_err(std::move(res_err))
{
}


template<typename T>
void result<T>::check_is_ok() const
{
    if (!is_ok()) {
        throw std::logic_error("reading async result value wich contains error");
    }
}


template<typename T>
bool operator==(const result<T> &lhs, const result<T> &rhs)
{
    return
        (!lhs.is_ok() && !rhs.is_ok() && lhs.get_error() == rhs.get_error())
        || (lhs.is_ok() && rhs.is_ok() && lhs.get_value() == rhs.get_value());
}


template<typename T>
bool operator!=(const result<T> &lhs, const result<T> &rhs)
{
    return !(lhs == rhs);
}


template<typename T>
result<std::decay_t<T>> make_ok_result(T &&value)
{
    return result<std::decay_t<T>>::create_with_value(std::forward<T>(value));
}


template<typename T>
result<T> make_error_result(std::error_code ec)
{
    return result<T>::create_with_error(ec);
}


template<typename T>
result<T> make_error_result(std::errc e)
{
    return result<T>::create_with_error(std::make_error_code(e));
}

}

#endif /* NOSYNC__RESULT_IMPL_H */
