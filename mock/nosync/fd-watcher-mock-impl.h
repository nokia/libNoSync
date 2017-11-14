// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_WATCHER_MOCK_IMPL_H
#define NOSYNC__FD_WATCHER_MOCK_IMPL_H

#include <utility>


namespace nosync
{

inline std::unique_ptr<activity_handle> fd_watcher_no_rvref_wrapper::add_watch(int fd, fd_watch_mode mode, std::function<void()> &&notify_func)
{
    return add_watch_impl(fd, mode, std::move(notify_func));
}

}

#endif /* NOSYNC__FD_WATCHER_MOCK_IMPL_H */
