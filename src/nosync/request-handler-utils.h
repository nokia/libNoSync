// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__REQUEST_HANDLER_UTILS_H
#define NOSYNC__REQUEST_HANDLER_UTILS_H

#include <chrono>
#include <memory>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <nosync/result-handler.h>
#include <vector>


namespace nosync
{

template<typename Req, typename Res>
void handle_requests_sequentially(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&req_handler,
    std::vector<Req> &&requests, std::chrono::nanoseconds req_timeout,
    result_handler<std::vector<Res>> &&res_handler);

}

#include <nosync/request-handler-utils-impl.h>

#endif /* NOSYNC__REQUEST_HANDLER_UTILS_H */
