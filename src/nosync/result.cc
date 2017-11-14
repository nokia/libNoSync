// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/result.h>
#include <stdexcept>
#include <utility>

using std::error_code;
using std::get;
using std::move;
using std::tuple;


namespace nosync
{

result<void> result<void>::create_with_value()
{
    return {{true, error_code()}};
}


result<void> result<void>::create_with_error(error_code ec)
{
    return {{false, ec}};
}


bool result<void>::is_ok() const
{
    return get<bool>(res_err);
}


error_code result<void>::get_error() const
{
    if (is_ok()) {
        throw std::logic_error("reading error from async result which contains value");
    }

    return get<error_code>(res_err);
}


result<void>::result(tuple<bool, error_code> res_err)
    : res_err(move(res_err))
{
}


bool operator==(const result<void> &lhs, const result<void> &rhs)
{
    return
        (!lhs.is_ok() && !rhs.is_ok() && lhs.get_error() == rhs.get_error())
        || (lhs.is_ok() && rhs.is_ok());
}


bool operator!=(const result<void> &lhs, const result<void> &rhs)
{
    return !(lhs == rhs);
}


result<void> make_ok_result()
{
    return result<void>::create_with_value();
}

}
