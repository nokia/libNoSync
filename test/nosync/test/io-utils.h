// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC_TEST__IO_UTILS_H
#define NOSYNC_TEST__IO_UTILS_H

#include <cstddef>


namespace nosync
{
namespace test
{

int read_nointr(int fd, void *buf, std::size_t count);
int write_nointr(int fd, const void *buf, std::size_t count);

}
}

#endif /* NOSYNC_TEST__IO_UTILS_H */
