// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__REQUEST_HANDLER_UTILS_IMPL_H
#define NOSYNC__REQUEST_HANDLER_UTILS_IMPL_H

#include <algorithm>
#include <chrono>
#include <nosync/result-handler-utils.h>
#include <utility>


namespace nosync
{

namespace request_handler_utils_impl
{

template<typename Req, typename Res>
void handle_requests_sequentially_impl(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&req_handler,
    std::vector<Req> &&requests, std::size_t requests_start_index,
    std::chrono::nanoseconds req_timeout, result_handler<std::vector<Res>> &&res_handler)
{
    using std::move;

    if (requests_start_index == requests.size()) {
        invoke_result_handler_later(evloop, move(res_handler), make_ok_result(std::vector<Res>()));
        return;
    }

    auto head = move(requests[requests_start_index]);

    req_handler->handle_request(
        move(head), req_timeout,
        merge_result_handler<Res>(
            move(res_handler),
            [&evloop, req_handler = move(req_handler), requests = move(requests), requests_start_index, req_timeout](auto head_res_value, auto res_handler) mutable {
                handle_requests_sequentially_impl(
                    evloop, move(req_handler), move(requests), requests_start_index + 1, req_timeout,
                    transform_result_handler<std::vector<Res>>(
                        move(res_handler),
                        [head_res_value = move(head_res_value)](auto tail_res_value) mutable {
                            auto results = move(tail_res_value);
                            results.push_back(move(head_res_value));
                            return make_ok_result(move(results));
                        }));
            }));
}

}


template<typename Req, typename Res>
void handle_requests_sequentially(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&req_handler,
    std::vector<Req> &&requests, std::chrono::nanoseconds req_timeout,
    result_handler<std::vector<Res>> &&res_handler)
{
    request_handler_utils_impl::handle_requests_sequentially_impl(
        evloop, std::move(req_handler), std::move(requests), 0, req_timeout,
        transform_result_handler<std::vector<Res>>(
            std::move(res_handler),
            [](auto res_values) {
                std::reverse(res_values.begin(), res_values.end());
                return make_ok_result(std::move(res_values));
            }));
}

}

#endif /* NOSYNC__REQUEST_HANDLER_UTILS_IMPL_H */
