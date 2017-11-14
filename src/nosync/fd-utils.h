// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_UTILS_H
#define NOSYNC__FD_UTILS_H

#include <cstddef>
#include <experimental/string_view>
#include <nosync/result.h>
#include <string>


namespace nosync
{

/*!
Wrapper around POSIX read() that returns data as result<>.

The function automatically handles EINTR from system call (by retrying it).
*/
result<std::string> read_some_bytes_from_fd(int fd, std::size_t max_size);


/*!
Wrapper around POSIX write() that returns status as result<>.

The function automatically handles EINTR from system call (by retrying it).
*/
result<std::size_t> write_some_bytes_to_fd(int fd, std::experimental::string_view data);

}

#endif /* NOSYNC__FD_UTILS_H */
