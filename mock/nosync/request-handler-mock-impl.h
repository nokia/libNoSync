// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__REQUEST_IMPL_HANDLER_MOCK_IMPL_H
#define NOSYNC__REQUEST_IMPL_HANDLER_MOCK_IMPL_H

#include <utility>


namespace nosync
{

template<typename Req, typename Res>
void request_handler_no_rvref_wrapper<Req, Res>::handle_request(
    Req &&request, std::chrono::nanoseconds timeout,
    result_handler<Res> &&response_handler)
{
    handle_request_impl(std::move(request), timeout, std::move(response_handler));
}

}

#endif /* NOSYNC__REQUEST_IMPL_HANDLER_MOCK_IMPL_H */
