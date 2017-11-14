// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__PRIORITIZING_REQUEST_HANDLER_H
#define NOSYNC__PRIORITIZING_REQUEST_HANDLER_H

#include <array>
#include <cstddef>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <memory>


namespace nosync
{

/*!
Create request_handler<> proxies for handling requests at different priorities.

This function creates an array of request_handler proxies used for prioritizing
incoming requests. The returned array contains proxies with decreasing
priorities - proxy at the beginning has highest priority, proxy at the end has
lowest one.

Requests from proxies are passed to single underlying request_handler object
such that:
 - requests are processed sequentially - only one request is processed at a
time),
 - pending requests are prioritized - requests from single proxy are processed
in the order of receipt but when next request is to be forwarded to the
underlying request_handler and requests from multiple proxies are pending, the
one from highest priority proxy is always selected.
*/
template<typename Req, typename Res, std::size_t N = 2>
std::array<std::shared_ptr<request_handler<Req, Res>>, N> make_prioritizing_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler);

}

#include <nosync/prioritizing-request-handler-impl.h>

#endif /* NOSYNC__PRIORITIZING_REQUEST_HANDLER_H */
