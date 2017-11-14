// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_READER_MOCK_IMPL_H
#define NOSYNC__BYTES_READER_MOCK_IMPL_H

#include <utility>


namespace nosync
{

inline void bytes_reader_no_rvref_wrapper::read_some_bytes(
    std::size_t max_size, std::chrono::nanoseconds timeout,
    result_handler<std::string> &&res_handler)
{
    read_some_bytes_impl(max_size, timeout, std::move(res_handler));
}

}

#endif /* NOSYNC__BYTES_READER_MOCK_IMPL_H */
