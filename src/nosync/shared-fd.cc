// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/shared-fd.h>
#include <utility>


namespace nosync
{

using std::make_shared;
using std::move;


shared_fd::shared_fd()
    : ofd(make_shared<owned_fd>())
{ }


shared_fd::shared_fd(int fd)
    : ofd(make_shared<owned_fd>(fd))
{ }


shared_fd::shared_fd(owned_fd &&ofd)
    : ofd(make_shared<owned_fd>(move(ofd)))
{ }


shared_fd::operator bool() const noexcept
{
    return static_cast<bool>(*ofd);
}


int shared_fd::operator*() const noexcept
{
    return **ofd;
}

}
