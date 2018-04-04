// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_HANDLER_UTILS_IMPL_H
#define NOSYNC__RESULT_HANDLER_UTILS_IMPL_H

#include <nosync/event-loop-utils.h>


namespace nosync
{

template<typename T>
void invoke_result_handler_at(
    event_loop &evloop, std::chrono::time_point<eclock> time,
    result_handler<T> &&res_handler, result<T> &&res)
{
    evloop.invoke_at(
        time,
        [res_handler = std::move(res_handler), res = std::move(res)]() mutable {
            res_handler(std::move(res));
        });
}


template<typename T>
void invoke_result_handler_later(event_loop &evloop, result_handler<T> &&res_handler, result<T> &&res)
{
    invoke_later(
        evloop,
        [res_handler = std::move(res_handler), res = std::move(res)]() mutable {
            res_handler(std::move(res));
        });
}

}

#endif /* NOSYNC__RESULT_HANDLER_UTILS_IMPL_H */
