// This file is part of libnosync library. See LICENSE file for license details.
#include <array>
#include <cerrno>
#include <cstddef>
#include <memory>
#include <nosync/fd-utils.h>
#include <nosync/result-utils.h>
#include <system_error>
#include <unistd.h>

using std::array;
using std::experimental::string_view;
using std::make_unique;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

constexpr auto max_local_buffer_size = 8192U;


result<string> read_some_bytes_from_fd(int fd, size_t max_size, char *tmp_buffer)
{
    ssize_t read_res;
    do {
        read_res = ::read(fd, tmp_buffer, max_size);
    } while (read_res == -1 && errno == EINTR);

    auto res =
        read_res >= 0
            ? make_ok_result(string(tmp_buffer, static_cast<size_t>(read_res)))
            : make_raw_error_result_from_errno();

    return res;
}

}


result<string> read_some_bytes_from_fd(int fd, size_t max_size)
{
    return read_some_bytes_from_fd(
        fd, max_size,
        max_size <= max_local_buffer_size
            ? array<char, max_local_buffer_size>().data()
            : make_unique<char[]>(max_size).get());
}


result<size_t> write_some_bytes_to_fd(int fd, string_view data)
{
    ssize_t write_res;
    do {
        write_res = ::write(fd, data.data(), data.size());
    } while (write_res == -1 && errno == EINTR);

    auto res =
        write_res >= 0
            ? make_ok_result(static_cast<size_t>(write_res))
            : make_raw_error_result_from_errno();

    return res;
}

}
