// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__CALL_ONCE_REQUEST_HANDLER_IMPL_H
#define NOSYNC__CALL_ONCE_REQUEST_HANDLER_IMPL_H

#include <nosync/func-request-handler.h>


namespace nosync
{

template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_call_once_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler,
    std::error_code other_calls_error)
{
    using std::move;

    return make_func_request_handler<Req, Res>(
        [&evloop, base_req_handler = move(base_req_handler), other_calls_error](auto req, auto timeout, auto res_handler) mutable {
            if (base_req_handler) {
                auto tmp_req_handler = move(base_req_handler);
                base_req_handler = nullptr;
                tmp_req_handler->handle_request(move(req), timeout, move(res_handler));
            } else {
                invoke_result_handler_later(
                    evloop, move(res_handler), make_error_result<Res>(other_calls_error));
            }
        });

}

}

#endif /* NOSYNC__CALL_ONCE_REQUEST_HANDLER_IMPL_H */
