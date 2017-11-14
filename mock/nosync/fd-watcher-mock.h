// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_WATCHER_MOCK_H
#define NOSYNC__FD_WATCHER_MOCK_H

#include <gmock/gmock.h>
#include <nosync/fd-watcher.h>


namespace nosync
{

class fd_watcher_no_rvref_wrapper : public fd_watcher
{
public:
    fd_watcher_no_rvref_wrapper() = default;

    std::unique_ptr<activity_handle> add_watch(int fd, fd_watch_mode mode, std::function<void()> &&notify_func) override final;

    virtual std::unique_ptr<activity_handle> add_watch_impl(int fd, fd_watch_mode mode, std::function<void()> notify_func) = 0;
};


class fd_watcher_mock : public fd_watcher_no_rvref_wrapper
{
public:
    fd_watcher_mock() = default;

    MOCK_METHOD3(add_watch_impl, std::unique_ptr<activity_handle>(int, fd_watch_mode, std::function<void()>));
};

}

#include <nosync/fd-watcher-mock-impl.h>

#endif /* NOSYNC__FD_WATCHER_MOCK_H */
