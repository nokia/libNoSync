// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RAW_ERROR_RESULT_IMPL_H
#define NOSYNC__RAW_ERROR_RESULT_IMPL_H


namespace nosync
{

inline raw_error_result::raw_error_result(std::error_code ec)
    : ec(ec)
{
}


inline raw_error_result::raw_error_result(std::errc e)
    : ec(make_error_code(e))
{
}


template<typename T>
raw_error_result::raw_error_result(const result<T> &res)
    : ec(res.get_error())
{
}


template<typename T>
result<T> raw_error_result::as_result() const
{
    return result<T>::create_with_error(ec);
}


template<typename T>
raw_error_result::operator result<T>() const
{
    return as_result<T>();
}

}

#endif /* NOSYNC__RAW_ERROR_RESULT_IMPL_H */
