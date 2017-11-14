// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__REQUESTS_QUEUE_H
#define NOSYNC__REQUESTS_QUEUE_H

#include <chrono>
#include <cstddef>
#include <deque>
#include <experimental/optional>
#include <functional>
#include <nosync/eclock.h>
#include <nosync/event-loop.h>
#include <nosync/result-handler.h>
#include <memory>
#include <string>
#include <tuple>


namespace nosync
{

/*!
Helper class template for implementing queues of async requests.

This class template is to be used by request_handler implementations and similar
classes which must store incoming requests in a queue and use them later in the
same order e.g. to ensure sequential processing, throttling, etc.

Besides just storing the requests, the class automatically handles timeouts for
them. If timeout is reached for a request, timeout error is passed to its
response handler and the request is removed from the queue.
*/
template<typename Req, typename Res>
class requests_queue
{
public:
    explicit requests_queue(event_loop &evloop);
    ~requests_queue();

    void push_request(
        Req &&request, std::chrono::time_point<eclock> timeout_end,
        result_handler<Res> &&res_handler);

    bool has_requests() const;

    std::tuple<Req, std::chrono::time_point<eclock>, result_handler<Res>> pull_next_request();

private:
    void handle_pending_timeouts();
    void disable_timeout_task_if_present();
    void reschedule_timeout_task();

    event_loop &evloop;
    std::deque<std::tuple<Req, std::chrono::time_point<eclock>, result_handler<Res>>> requests;
    std::experimental::optional<std::tuple<std::chrono::time_point<eclock>, std::unique_ptr<activity_handle>>> scheduled_timeout_task;
};

}

#include <nosync/requests-queue-impl.h>

#endif /* NOSYNC__REQUESTS_QUEUE_H */
