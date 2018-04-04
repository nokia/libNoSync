// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/owned-fd.h>
#include <unistd.h>
#include <utility>


namespace nosync
{

namespace
{

void destroy_fd(int &fd) noexcept
{
    if (fd >= 0) {
        (void) close(fd);
        fd = -1;
    }
}

}


owned_fd::owned_fd() noexcept
    : fd(-1)
{ }


owned_fd::owned_fd(int fd) noexcept
    : fd(fd)
{ }


owned_fd::~owned_fd()
{
    destroy_fd(fd);
}


owned_fd::owned_fd(owned_fd &&other) noexcept
    : fd(other.release())
{ }


owned_fd &owned_fd::operator=(owned_fd &&other) noexcept
{
    if (&other != this) {
        destroy_fd(fd);
        fd = other.release();
    }

    return *this;
}


void owned_fd::reset() noexcept
{
    destroy_fd(fd);
}


int owned_fd::release() noexcept
{
    auto saved_fd = fd;
    fd = -1;

    return saved_fd;
}


owned_fd::operator bool() const noexcept
{
    return fd >= 0;
}


int owned_fd::operator*() const noexcept
{
    return fd;
}

}
