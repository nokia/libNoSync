// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__THREADED_REQUEST_HANDLER_H
#define NOSYNC__THREADED_REQUEST_HANDLER_H

#include <chrono>
#include <functional>
#include <nosync/request-handler.h>
#include <nosync/result.h>
#include <memory>


namespace nosync
{

template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_threaded_request_handler(
    std::function<void(std::function<void()>)> &&evloop_executor,
    std::function<void(std::function<void()>)> &&thread_executor,
    std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> &&sync_req_handler);

}

#include <nosync/threaded-request-handler-impl.h>

#endif /* NOSYNC__THREADED_REQUEST_HANDLER_H */
