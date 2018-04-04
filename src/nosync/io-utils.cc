// This file is part of libnosync library. See LICENSE file for license details.
#include <cstddef>
#include <fcntl.h>
#include <nosync/exceptions.h>
#include <nosync/io-utils.h>
#include <nosync/net-utils.h>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

using namespace std::string_literals;
using std::array;
using std::move;
using std::string;
using std::system_category;
using std::system_error;


namespace nosync
{

owned_fd open_abstract_listening_stream_socket(const string &abstract_path, int conn_backlog_size)
{
    auto sock_res = open_local_abstract_listening_stream_socket(abstract_path, conn_backlog_size);
    if (!sock_res.is_ok()) {
        throw system_error(sock_res.get_error().value(), system_category());
    }

    return move(sock_res.get_value());
}


array<owned_fd, 2> create_nonblocking_pipe()
{
    int pipe_fds[2];
    int pipe_retval = ::pipe2(pipe_fds, O_CLOEXEC | O_NONBLOCK);
    if (pipe_retval < 0) {
        throw_system_error_from_errno("failed to create pipe");
    }

    return {owned_fd(pipe_fds[0]), owned_fd(pipe_fds[1])};
}

}
