// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__CALL_ONCE_REQUEST_HANDLER_H
#define NOSYNC__CALL_ONCE_REQUEST_HANDLER_H

#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <nosync/result.h>
#include <memory>
#include <system_error>


namespace nosync
{

template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_call_once_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler,
    std::error_code other_calls_error = std::make_error_code(std::errc::connection_aborted));

}

#include <nosync/call-once-request-handler-impl.h>

#endif /* NOSYNC__CALL_ONCE_REQUEST_HANDLER_H */
