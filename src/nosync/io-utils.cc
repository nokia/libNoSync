// This file is part of libnosync library. See LICENSE file for license details.
#include <fcntl.h>
#include <nosync/exceptions.h>
#include <nosync/io-utils.h>
#include <system_error>
#include <unistd.h>

using std::array;


namespace nosync
{

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
