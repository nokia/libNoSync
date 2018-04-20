// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__GROUPING_NULL_REQUEST_HANDLER_IMPL_H
#define NOSYNC__GROUPING_NULL_REQUEST_HANDLER_IMPL_H

#include <nosync/memory-utils.h>
#include <nosync/requests-queue.h>
#include <nosync/result-handler.h>
#include <nosync/time-utils.h>
#include <utility>
#include <vector>


namespace nosync
{

namespace grouping_null_request_handler_impl
{

template<typename Res>
class grouping_null_request_handler : public request_handler<std::nullptr_t, Res>, public std::enable_shared_from_this<grouping_null_request_handler<Res>>
{
public:
    grouping_null_request_handler(event_loop &evloop, std::shared_ptr<request_handler<std::nullptr_t, Res>> &&base_req_handler);

    void handle_request(std::nullptr_t &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&res_handler) override;

private:
    event_loop &evloop;
    std::shared_ptr<request_handler<std::nullptr_t, Res>> base_req_handler;
    requests_queue<std::nullptr_t, Res> pending_requests;
    bool request_ongoing;
};


template<typename Res>
grouping_null_request_handler<Res>::grouping_null_request_handler(event_loop &evloop, std::shared_ptr<request_handler<std::nullptr_t, Res>> &&base_req_handler)
    : evloop(evloop), base_req_handler(std::move(base_req_handler)), pending_requests(evloop), request_ongoing(false)
{
}


template<typename Res>
void grouping_null_request_handler<Res>::handle_request(std::nullptr_t &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&res_handler)
{
    if (request_ongoing) {
        pending_requests.push_request(
            std::move(request), time_point_sat_add(evloop.get_etime(), timeout), std::move(res_handler));
        return;
    }

    request_ongoing = true;

    base_req_handler->handle_request(
        std::move(request), timeout,
        [req_handler_wptr = weak_from_that(this), res_handler = std::move(res_handler)](auto res) mutable {
            std::vector<std::tuple<std::nullptr_t, std::chrono::time_point<eclock>, result_handler<Res>>> grouped_reqs;

            auto req_handler_ptr = req_handler_wptr.lock();
            if (req_handler_ptr) {
                while (req_handler_ptr->pending_requests.has_requests()) {
                    grouped_reqs.push_back(req_handler_ptr->pending_requests.pull_next_request());
                }
                req_handler_ptr->request_ongoing = false;
            }

            res_handler(res);
            res_handler = nullptr;

            for (auto &req : grouped_reqs) {
                auto &pending_res_handler = std::get<result_handler<Res>>(req);
                pending_res_handler(res);
                pending_res_handler = nullptr;
            }
        });
}

}


template<typename Res>
std::shared_ptr<request_handler<std::nullptr_t, Res>> make_grouping_null_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<std::nullptr_t, Res>> &&base_req_handler)
{
    return std::make_shared<grouping_null_request_handler_impl::grouping_null_request_handler<Res>>(
        evloop, std::move(base_req_handler));
}

}

#endif /* NOSYNC__GROUPING_NULL_REQUEST_HANDLER_IMPL_H */
