// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TRANSFORMING_REQUEST_HANDLER_H
#define NOSYNC__TRANSFORMING_REQUEST_HANDLER_H

#include <functional>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <nosync/result.h>
#include <memory>


namespace nosync
{

/*!
Create request_handler<> proxy that transforms requests on the fly.

The created request_handler<> transforms each request using provided function
before passing it to the underlying request_handler implementation. Transformed
value is provided as a result<> object which allows signalling transformation
failures. If transformation fails, the error is passed (asynchronously) from
the adapter to the result handler without using the underlying request_handler
at all.
*/
template<typename InReq, typename OutReq, typename Res>
std::shared_ptr<request_handler<InReq, Res>> make_transforming_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<OutReq, Res>> &&base_req_handler,
    std::function<result<OutReq>(InReq)> &&req_transformer);


/*!
Create request_handler<> proxy that transforms requests and responses on the fly.

This is extension of make_transforming_request_handler() which supports also
transforming results received from base_req_handler.
*/
template<typename InReq, typename OutReq, typename InRes, typename OutRes>
std::shared_ptr<request_handler<InReq, OutRes>> make_full_transforming_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<OutReq, InRes>> &&base_req_handler,
    std::function<result<OutReq>(InReq)> &&req_transformer,
    std::function<result<OutRes>(InRes)> &&res_transformer);

}

#include <nosync/transforming-request-handler-impl.h>

#endif /* NOSYNC__TRANSFORMING_REQUEST_HANDLER_H */
