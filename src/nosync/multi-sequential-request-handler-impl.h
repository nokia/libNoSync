// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__MULTI_SEQUENTIAL_REQUEST_HANDLER_IMPL_H
#define NOSYNC__MULTI_SEQUENTIAL_REQUEST_HANDLER_IMPL_H

#include <map>
#include <nosync/multi-sequential-request-handler.h>
#include <nosync/sequential-request-handler.h>
#include <nosync/type-utils.h>
#include <tuple>


namespace nosync
{

namespace multi_sequential_request_handler_impl
{

template<typename Req, typename Res, typename K>
class multi_sequential_request_handler : public request_handler<Req, Res>
{
public:
    multi_sequential_request_handler(
        event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler,
        std::function<K(const Req &)> &&sequence_key_func);

    void handle_request(
        Req &&request, std::chrono::nanoseconds timeout,
        result_handler<Res> &&res_handler) override;

private:
    event_loop &evloop;
    std::shared_ptr<request_handler<Req, Res>> base_req_handler;
    std::function<K(const Req &)> sequence_key_func;
    std::map<K, std::shared_ptr<request_handler<Req, Res>>> sub_req_handlers;
};


template<typename Req, typename Res, typename K>
multi_sequential_request_handler<Req, Res, K>::multi_sequential_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler,
    std::function<K(const Req &)> &&sequence_key_func)
    : evloop(evloop), base_req_handler(std::move(base_req_handler)),
    sequence_key_func(std::move(sequence_key_func)), sub_req_handlers()
{
}


template<typename Req, typename Res, typename K>
void multi_sequential_request_handler<Req, Res, K>::handle_request(
    Req &&request, std::chrono::nanoseconds timeout, result_handler<Res> &&res_handler)
{
    const auto sequence_key = sequence_key_func(request);
    auto sub_req_handler_iter = sub_req_handlers.find(sequence_key);
    if (sub_req_handler_iter == sub_req_handlers.end()) {
        std::tie(sub_req_handler_iter, std::ignore) = sub_req_handlers.emplace(
            sequence_key, make_sequential_request_handler(evloop, make_copy(base_req_handler)));
    }

    // TODO: remove unused sub-handlers
    sub_req_handler_iter->second->handle_request(
        std::move(request), timeout, std::move(res_handler));
}

}


template<typename Req, typename Res, typename K>
std::shared_ptr<request_handler<Req, Res>> make_multi_sequential_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<Req, Res>> &&base_req_handler,
    std::function<K(const Req &)> &&sequence_key_func)
{
    return std::make_shared<multi_sequential_request_handler_impl::multi_sequential_request_handler<Req, Res, K>>(
        evloop, std::move(base_req_handler), std::move(sequence_key_func));
}

}

#endif /* NOSYNC__MULTI_SEQUENTIAL_REQUEST_HANDLER_IMPL_H */
