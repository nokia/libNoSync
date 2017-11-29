// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__IO_UTILS_H
#define NOSYNC__IO_UTILS_H

#include <array>
#include <nosync/owned-fd.h>


namespace nosync
{

std::array<owned_fd, 2> create_nonblocking_pipe();

}

#endif /* NOSYNC__IO_UTILS_H */
