// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__MULTI_SEQUENTIAL_REQUEST_HANDLER_H
#define NOSYNC__MULTI_SEQUENTIAL_REQUEST_HANDLER_H

#include <functional>
#include <memory>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>


namespace nosync
{

template<typename Req, typename Res, typename K>
std::shared_ptr<request_handler<Req, Res>> make_multi_sequential_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler,
    std::function<K(const Req &)> &&sequence_key_func);

}

#include <nosync/multi-sequential-request-handler-impl.h>

#endif /* NOSYNC__MULTI_SEQUENTIAL_REQUEST_HANDLER_H */
