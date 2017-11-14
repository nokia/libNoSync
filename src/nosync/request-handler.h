// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__REQUEST_HANDLER_H
#define NOSYNC__REQUEST_HANDLER_H

#include <chrono>
#include <functional>
#include <nosync/result.h>
#include <nosync/interface-type.h>
#include <nosync/result-handler.h>


namespace nosync
{

/*!
Base class template for async request–response implementations.

Generic interface template to be used for implementing asynchronous
request–response message exchange pattern (with types of request and response
as template parameters). The result is passed as result object holding the
actual response or error information.

An request_handler implementation:
 - must asynchronously invoke corresponding response_handler exactly once for
each handle_request() call,
 - must use best-effort approach to handling timeout value specified for a
response (some implementations may not support timeout below some minimum
value for example), when the timeout passes then the resulting result object
must hold errc::timed_out error, as if it was created with
std::make_error_code(std::errc::timed_out).
*/
template<typename Req, typename Res>
class request_handler : public interface_type
{
public:
    virtual void handle_request(
        Req &&request, std::chrono::nanoseconds timeout,
        result_handler<Res> &&response_handler) = 0;
};

}

#endif /* NOSYNC__REQUEST_HANDLER_H */
