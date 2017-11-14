// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SEQUENTIAL_REQUEST_HANDLER_H
#define NOSYNC__SEQUENTIAL_REQUEST_HANDLER_H

#include <memory>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>


namespace nosync
{

template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_sequential_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler);

}

#include <nosync/sequential-request-handler-impl.h>

#endif /* NOSYNC__SEQUENTIAL_REQUEST_HANDLER_H */
