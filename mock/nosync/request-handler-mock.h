// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__REQUEST_HANDLER_MOCK_H
#define NOSYNC__REQUEST_HANDLER_MOCK_H

#include <gmock/gmock.h>
#include <nosync/request-handler.h>


namespace nosync
{

template<typename Req, typename Res>
class request_handler_no_rvref_wrapper : public request_handler<Req, Res>
{
public:
    request_handler_no_rvref_wrapper() = default;

    void handle_request(
        Req &&request, std::chrono::nanoseconds timeout,
        result_handler<Res> &&response_handler) override final;

    virtual void handle_request_impl(
        Req request, std::chrono::nanoseconds timeout,
        result_handler<Res> response_handler) = 0;
};


template<typename Req, typename Res>
class request_handler_mock : public request_handler_no_rvref_wrapper<Req, Res>
{
public:
    request_handler_mock() = default;

    MOCK_METHOD3_T(handle_request_impl, void(Req, std::chrono::nanoseconds, std::function<void(result<Res>)>));
};

}

#include <nosync/request-handler-mock-impl.h>

#endif /* NOSYNC__REQUEST_HANDLER_MOCK_H */
