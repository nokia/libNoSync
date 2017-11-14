// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SEQUENTIAL_REQUEST_IMPL_HANDLER_IMPL_H
#define NOSYNC__SEQUENTIAL_REQUEST_IMPL_HANDLER_IMPL_H

#include <nosync/memory-utils.h>
#include <nosync/requests-queue.h>
#include <nosync/sequential-request-handler.h>
#include <nosync/time-utils.h>
#include <utility>


namespace nosync
{

namespace sequential_request_handler_impl
{

template<typename Req, typename Res>
class sequential_request_handler : public request_handler<Req, Res>, public std::enable_shared_from_this<sequential_request_handler<Req, Res>>
{
public:
    sequential_request_handler(event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler);

    void handle_request(Req &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&res_handler) override;

private:
    void handle_next_pending_request_if_needed();

    event_loop &evloop;
    std::shared_ptr<request_handler<Req, Res>> base_req_handler;
    requests_queue<Req, Res> pending_requests;
    bool request_ongoing;
};


template<typename Req, typename Res>
sequential_request_handler<Req, Res>::sequential_request_handler(event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler)
    : evloop(evloop), base_req_handler(std::move(base_req_handler)), pending_requests(evloop), request_ongoing(false)
{
}


template<typename Req, typename Res>
void sequential_request_handler<Req, Res>::handle_request(Req &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&res_handler)
{
    if (!request_ongoing) {
        base_req_handler->handle_request(
            std::move(request), timeout,
            [req_handler_wptr = weak_from_that(this), res_handler = std::move(res_handler)](auto res) {
                res_handler(std::move(res));
                auto req_handler_ptr = req_handler_wptr.lock();
                if (req_handler_ptr) {
                    req_handler_ptr->request_ongoing = false;
                    req_handler_ptr->handle_next_pending_request_if_needed();
                }
            });
        request_ongoing = true;
    } else {
        pending_requests.push_request(
            std::move(request), time_point_sat_add(evloop.get_etime(), timeout), std::move(res_handler));
    }
}


template<typename Req, typename Res>
void sequential_request_handler<Req, Res>::handle_next_pending_request_if_needed()
{
    if (!pending_requests.has_requests()) {
        return;
    }

    auto req_pack = pending_requests.pull_next_request();

    handle_request(
        std::move(std::get<0>(req_pack)),
        std::max(std::get<1>(req_pack) - evloop.get_etime(), std::chrono::nanoseconds(0)),
        std::move(std::get<2>(req_pack)));
}


}


template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_sequential_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler)
{
    return std::make_shared<sequential_request_handler_impl::sequential_request_handler<Req, Res>>(evloop, std::move(base_req_handler));
}

}

#endif /* NOSYNC__SEQUENTIAL_REQUEST_IMPL_HANDLER_IMPL_H */
