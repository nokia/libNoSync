// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__GROUPING_NULL_REQUEST_HANDLER_H
#define NOSYNC__GROUPING_NULL_REQUEST_HANDLER_H

#include <cstddef>
#include <memory>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>


namespace nosync
{

/*!
Create request_handler<nullptr, *> decorator for satisfying groups of concurrent requests with copies of single result.
*/
template<typename Res>
std::shared_ptr<request_handler<std::nullptr_t, Res>> make_grouping_null_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<std::nullptr_t, Res>> &&base_req_handler);

}

#include <nosync/grouping-null-request-handler-impl.h>

#endif /* NOSYNC__GROUPING_NULL_REQUEST_HANDLER_H */
