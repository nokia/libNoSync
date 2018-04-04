// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FUNC_REQUEST_HANDLER_H
#define NOSYNC__FUNC_REQUEST_HANDLER_H

#include <chrono>
#include <functional>
#include <nosync/request-handler.h>
#include <nosync/result-handler.h>
#include <memory>


namespace nosync
{

/*!
Adapt std::function<> to the corresponding request_handler<>.
*/
template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_func_request_handler(
    std::function<void(Req &&, std::chrono::nanoseconds, result_handler<Res> &&)> &&handle_func);

}

#include <nosync/func-request-handler-impl.h>

#endif /* NOSYNC__FUNC_REQUEST_HANDLER_H */
