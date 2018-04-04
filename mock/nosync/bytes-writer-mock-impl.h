// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_WRITER_MOCK_IMPL_H
#define NOSYNC__BYTES_WRITER_MOCK_IMPL_H

#include <utility>


namespace nosync
{

inline void bytes_writer_no_rvref_wrapper::write_bytes(std::string &&data, result_handler<void> &&res_handler)
{
    write_bytes_impl(std::move(data), std::move(res_handler));
}

}

#endif /* NOSYNC__BYTES_WRITER_MOCK_IMPL_H */
