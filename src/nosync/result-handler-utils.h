// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_HANDLER_UTILS_H
#define NOSYNC__RESULT_HANDLER_UTILS_H

#include <nosync/eclock.h>
#include <nosync/event-loop.h>
#include <nosync/result-handler.h>


namespace nosync
{

template<typename T>
void invoke_result_handler_at(
    event_loop &evloop, std::chrono::time_point<eclock> time,
    result_handler<T> &&res_handler, result<T> &&res);

template<typename T, typename ResultLike>
void invoke_result_handler_at(
    event_loop &evloop, std::chrono::time_point<eclock> time,
    result_handler<T> &&res_handler, const ResultLike &res);

template<typename T>
void invoke_result_handler_later(event_loop &evloop, result_handler<T> &&res_handler, result<T> &&res);

template<typename T, typename ResultLike>
void invoke_result_handler_later(event_loop &evloop, result_handler<T> &&res_handler, const ResultLike &res);

}

#include <nosync/result-handler-utils-impl.h>

#endif /* NOSYNC__RESULT_HANDLER_UTILS_H */
