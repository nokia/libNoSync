// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_H
#define NOSYNC__RESULT_H

#include <experimental/optional>
#include <system_error>
#include <tuple>
#include <type_traits>


namespace nosync
{

/*!
Variant-like class template for storing either value of an operation or error information.

Class template for storing the result of asynchronous operation as either:
 - the actual (return) value (of type T),
 - information about error as std::error_code (value T is not available in this case).

Conceptually it's variant type which holds either T or std::error_code.

Instances of the class can be compared using "==" and "!=" operators.

It's recommended to create result instances using make_* functions.
*/
template<typename T>
class result final
{
public:
    static result create_with_value(const T &value);
    static result create_with_value(T &&value);
    static result create_with_error(std::error_code ec);

    bool is_ok() const;

    T &get_value() &;
    const T &get_value() const &;
    T &&get_value() &&;

    std::error_code get_error() const;

    template<typename U>
    operator result<U>() const &;

    template<typename U>
    operator result<U>() &&;

private:
    result(std::tuple<std::experimental::optional<T>, std::error_code> res_err);

    void check_is_ok() const;

    std::tuple<std::experimental::optional<T>, std::error_code> res_err;
};


template<>
class result<void> final
{
public:
    static result create_with_value();
    static result create_with_error(std::error_code ec);

    bool is_ok() const;

    std::error_code get_error() const;

private:
    result(std::tuple<bool, std::error_code> res_err);

    std::tuple<bool, std::error_code> res_err;
};


template<typename T>
bool operator==(const result<T> &lhs, const result<T> &rhs);

template<typename T>
bool operator!=(const result<T> &lhs, const result<T> &rhs);

template<typename T>
result<std::decay_t<T>> make_ok_result(T &&value);

template<typename T>
result<T> make_error_result(std::error_code ec);

template<typename T>
result<T> make_error_result(std::errc e);


bool operator==(const result<void> &lhs, const result<void> &rhs);

bool operator!=(const result<void> &lhs, const result<void> &rhs);

result<void> make_ok_result();


}

#include <nosync/result-impl.h>

#endif /* NOSYNC__RESULT_H */
