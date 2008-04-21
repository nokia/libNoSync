// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RAW_ERROR_RESULT_H
#define NOSYNC__RAW_ERROR_RESULT_H

#include <nosync/result.h>
#include <system_error>


namespace nosync
{

/*!
Convenience class for building result<> objects with error information.

The class supports:
 - construction from any result<> object which contains an error (its is_ok()
returns false),
 - conversion to any result<> object.

The class lets you avoid boilerplate code in the following cases:
 - when creating result<> with specific error_code/errc value
("raw_error_result(ec)" instead of "make_error_result<blahblah>(ec)")
 - when creating result<> with error information from other result<>, possibly
with different contained type ("raw_error_result(other_res)" instead of
"make_error_result<blahblah>(other_res.get_error())")
... unless specific type of an expression is required e.g. for type deduction.
*/
class raw_error_result final
{
public:
    explicit raw_error_result(std::error_code ec);
    explicit raw_error_result(std::errc e);

    template<typename T>
    explicit raw_error_result(const result<T> &res);

    template<typename T>
    result<T> as_result() const;

    template<typename T>
    operator result<T>() const;

private:
    std::error_code ec;
};

}

#include <nosync/raw-error-result-impl.h>

#endif /* NOSYNC__RAW_ERROR_RESULT_H */
