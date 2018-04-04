// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_UTILS_IMPL_H
#define NOSYNC__RESULT_UTILS_IMPL_H


namespace nosync
{

template<typename T>
result<T> make_timeout_error_result()
{
    return result<T>::create_with_error(std::make_error_code(std::errc::timed_out));
}

}

#endif /* NOSYNC__RESULT_UTILS_IMPL_H */
