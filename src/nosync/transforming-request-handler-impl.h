// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TRANSFORMING_REQUEST_HANDLER_IMPL_H
#define NOSYNC__TRANSFORMING_REQUEST_HANDLER_IMPL_H

#include <nosync/result-handler-utils.h>


namespace nosync
{

namespace transforming_request_handler_impl
{

template<typename InReq, typename OutReq, typename InRes, typename OutRes>
class full_transforming_request_handler : public request_handler<InReq, OutRes>
{
public:
    full_transforming_request_handler(
        event_loop &evloop, std::shared_ptr<request_handler<OutReq, InRes>> &&base_req_handler,
        std::function<result<OutReq>(InReq)> &&req_transformer,
        std::function<result<OutRes>(InRes)> &&res_transformer);

    void handle_request(
        InReq &&request, std::chrono::nanoseconds timeout,
        result_handler<OutRes> &&response_handler) override;

private:
    event_loop &evloop;
    std::shared_ptr<request_handler<OutReq, InRes>> base_req_handler;
    std::function<result<OutReq>(InReq)> req_transformer;
    std::function<result<OutRes>(InRes)> res_transformer;
};


template<typename InReq, typename OutReq, typename InRes, typename OutRes>
full_transforming_request_handler<InReq, OutReq, InRes, OutRes>::full_transforming_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<OutReq, InRes>> &&base_req_handler,
    std::function<result<OutReq>(InReq)> &&req_transformer, std::function<result<OutRes>(InRes)> &&res_transformer)
    : evloop(evloop), base_req_handler(std::move(base_req_handler)),
    req_transformer(std::move(req_transformer)), res_transformer(move(res_transformer))
{
}


template<typename InReq, typename OutReq, typename InRes, typename OutRes>
void full_transforming_request_handler<InReq, OutReq, InRes, OutRes>::handle_request(
    InReq &&request, std::chrono::nanoseconds timeout,
    result_handler<OutRes> &&response_handler)
{
    using std::move;

    auto req_trans_res = req_transformer(move(request));
    if (req_trans_res.is_ok()) {
        base_req_handler->handle_request(
            move(req_trans_res.get_value()), timeout,
            transform_result_handler<InRes>(
                move(response_handler),
                [this](auto resp) {
                    return res_transformer(move(resp));
                }));
    } else {
        invoke_result_handler_later(
            evloop, move(response_handler), make_error_result<OutRes>(req_trans_res.get_error()));
    }
}


template<typename InReq, typename OutReq, typename Res>
class transforming_request_handler : public request_handler<InReq, Res>
{
public:
    transforming_request_handler(
        event_loop &evloop, std::shared_ptr<request_handler<OutReq, Res>> &&base_req_handler,
        std::function<result<OutReq>(InReq)> &&req_transformer);

    void handle_request(
        InReq &&request, std::chrono::nanoseconds timeout,
        result_handler<Res> &&response_handler) override;

private:
    event_loop &evloop;
    std::shared_ptr<request_handler<OutReq, Res>> base_req_handler;
    std::function<result<OutReq>(InReq)> req_transformer;
};


template<typename InReq, typename OutReq, typename Res>
transforming_request_handler<InReq, OutReq, Res>::transforming_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<OutReq, Res>> &&base_req_handler,
    std::function<result<OutReq>(InReq)> &&req_transformer)
    : evloop(evloop), base_req_handler(std::move(base_req_handler)),
    req_transformer(std::move(req_transformer))
{
}


template<typename InReq, typename OutReq, typename Res>
void transforming_request_handler<InReq, OutReq, Res>::handle_request(
    InReq &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&response_handler)
{
    using std::move;

    auto req_trans_res = req_transformer(move(request));
    if (req_trans_res.is_ok()) {
        base_req_handler->handle_request(
            move(req_trans_res.get_value()), timeout,
            [response_handler = move(response_handler)](auto res) {
                response_handler(move(res));
            });
    } else {
        invoke_result_handler_later(
            evloop, move(response_handler), make_error_result<Res>(req_trans_res.get_error()));
    }
}

}


template<typename InReq, typename OutReq, typename Res>
std::shared_ptr<request_handler<InReq, Res>> make_transforming_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<OutReq, Res>> &&base_req_handler,
    std::function<result<OutReq>(InReq)> &&req_transformer)
{
    return std::make_shared<transforming_request_handler_impl::transforming_request_handler<InReq, OutReq, Res>>(
        evloop, std::move(base_req_handler), std::move(req_transformer));
}


template<typename InReq, typename OutReq, typename InRes, typename OutRes>
std::shared_ptr<request_handler<InReq, OutRes>> make_full_transforming_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<OutReq, InRes>> &&base_req_handler,
    std::function<result<OutReq>(InReq)> &&req_transformer,
    std::function<result<OutRes>(InRes)> &&res_transformer)
{
    return std::make_shared<transforming_request_handler_impl::full_transforming_request_handler<InReq, OutReq, InRes, OutRes>>(
        evloop, std::move(base_req_handler), std::move(req_transformer), std::move(res_transformer));
}

}

#endif /* NOSYNC__TRANSFORMING_REQUEST_HANDLER_IMPL_H */
