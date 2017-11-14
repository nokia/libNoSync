// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__THROTTLING_REQUEST_HANDLER_H
#define NOSYNC__THROTTLING_REQUEST_HANDLER_H

#include <chrono>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <memory>


namespace nosync
{

template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_throttling_request_handler(
    event_loop &evloop, std::chrono::nanoseconds min_req_time_distance,
    std::shared_ptr<request_handler<Req, Res>> &&base_handler);

}

#include <nosync/throttling-request-handler-impl.h>

#endif /* NOSYNC__THROTTLING_REQUEST_HANDLER_H */
