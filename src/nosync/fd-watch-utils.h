// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_WATCH_UTILS_H
#define NOSYNC__FD_WATCH_UTILS_H

#include <chrono>
#include <nosync/fd-watching-event-loop.h>
#include <nosync/result-handler.h>


namespace nosync
{

void watch_fd_with_timeout(
    fd_watching_event_loop &evloop, int fd, fd_watch_mode mode, std::chrono::nanoseconds timeout,
    result_handler<void> &&res_handler);

}

#endif /* NOSYNC__FD_WATCH_UTILS_H */
