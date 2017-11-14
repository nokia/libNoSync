// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_WATCHER_H
#define NOSYNC__FD_WATCHER_H

#include <functional>
#include <nosync/activity-handle.h>
#include <nosync/interface-type.h>
#include <memory>


namespace nosync
{

enum class fd_watch_mode
{
    input,
    output,
};


/*!
Interface for monitoring states of file descriptors.

This interface can be used for monitoring file descriptors and triggering
notifications when they are ready to read data from or write data to (depending
on "watch mode" used when adding new watch).

Any number of watches can be registered for the same file descriptor (both with
the same and with different watch modes).

For each registered watch new activity_handle object is created and returned to
the caller. It can be used to:
 - check if the watch is (still) present (is_enabled()),
 - remove the watch (disable()).
*/
class fd_watcher : public interface_type
{
public:
    virtual std::unique_ptr<activity_handle> add_watch(int fd, fd_watch_mode mode, std::function<void()> &&notify_func) = 0;
};

}

#endif /* NOSYNC__FD_WATCHER_H */
