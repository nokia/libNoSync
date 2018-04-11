// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__THREADED_REQUEST_HANDLER_IMPL_H
#define NOSYNC__THREADED_REQUEST_HANDLER_IMPL_H

#include <nosync/raw-error-result.h>
#include <nosync/result-handler.h>
#include <system_error>
#include <utility>


namespace nosync
{

namespace threaded_request_handler_impl
{

template<typename Req, typename Res>
struct thread_exec_context : public std::enable_shared_from_this<thread_exec_context<Req, Res>>
{
    thread_exec_context(
        std::function<void(std::function<void()>)> &&evloop_executor,
        std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> &&sync_req_handler);

    std::function<void(std::function<void()>)> evloop_executor;
    std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> sync_req_handler;
};


template<typename Req, typename Res>
class threaded_request_handler : public request_handler<Req, Res>
{
public:
    threaded_request_handler(
        std::function<void(std::function<void()>)> &&evloop_executor,
        std::function<void(std::function<void()>)> &&thread_executor,
        std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> &&sync_req_handler);

    void handle_request(
        Req &&request, std::chrono::nanoseconds timeout,
        result_handler<Res> &&response_handler) override;

private:
    std::function<void(std::function<void()>)> thread_executor;
    std::shared_ptr<thread_exec_context<Req, Res>> exec_ctx;
};


template<typename Req, typename Res>
thread_exec_context<Req, Res>::thread_exec_context(
    std::function<void(std::function<void()>)> &&evloop_executor,
    std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> &&sync_req_handler)
    : evloop_executor(std::move(evloop_executor)), sync_req_handler(std::move(sync_req_handler))
{
}


template<typename Req, typename Res>
threaded_request_handler<Req, Res>::threaded_request_handler(
    std::function<void(std::function<void()>)> &&evloop_executor,
    std::function<void(std::function<void()>)> &&thread_executor,
    std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> &&sync_req_handler)
    : thread_executor(std::move(thread_executor)),
    exec_ctx(std::make_shared<thread_exec_context<Req, Res>>(std::move(evloop_executor), std::move(sync_req_handler)))
{
}


template<typename Req, typename Res>
void threaded_request_handler<Req, Res>::handle_request(
    Req &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&response_handler)
{
    thread_executor(
        [exec_ctx = exec_ctx, request = std::make_shared<Req>(std::move(request)), timeout, response_handler = std::move(response_handler)]() mutable {
            std::shared_ptr<result<Res>> response;
            try {
                response = std::make_shared<result<Res>>(exec_ctx->sync_req_handler(std::move(*request), timeout));
            } catch (...) {
                response = std::make_shared<result<Res>>(raw_error_result(std::errc::owner_dead));
            }
            exec_ctx->evloop_executor(
                [response_handler = std::move(response_handler), response = std::move(response)]() mutable {
                    response_handler(std::move(*response));
                });
        });
}

}


template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_threaded_request_handler(
    std::function<void(std::function<void()>)> &&evloop_executor,
    std::function<void(std::function<void()>)> &&thread_executor,
    std::function<result<Res>(Req request, std::chrono::nanoseconds timeout)> &&sync_req_handler)
{
    return std::make_shared<threaded_request_handler_impl::threaded_request_handler<Req, Res>>(
        std::move(evloop_executor), std::move(thread_executor), std::move(sync_req_handler));
}

}

#endif /* NOSYNC__THREADED_REQUEST_HANDLER_IMPL_H */
