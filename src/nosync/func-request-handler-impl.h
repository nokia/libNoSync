// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FUNC_REQUEST_HANDLER_IMPL_H
#define NOSYNC__FUNC_REQUEST_HANDLER_IMPL_H

#include <utility>


namespace nosync
{

namespace func_request_handler_impl
{

template<typename Req, typename Res>
class func_request_handler : public request_handler<Req, Res>
{
public:
    func_request_handler(
        std::function<void(Req &&, std::chrono::nanoseconds, result_handler<Res> &&)> &&handle_func);

    void handle_request(
        Req &&request, std::chrono::nanoseconds timeout,
        result_handler<Res> &&response_handler) override;

private:
    std::function<void(Req &&, std::chrono::nanoseconds, result_handler<Res> &&)> handle_func;
};


template<typename Req, typename Res>
func_request_handler<Req, Res>::func_request_handler(
    std::function<void(Req &&, std::chrono::nanoseconds, result_handler<Res> &&)> &&handle_func)
    : handle_func(std::move(handle_func))
{
}


template<typename Req, typename Res>
void func_request_handler<Req, Res>::handle_request(
    Req &&request, std::chrono::nanoseconds timeout,
    result_handler<Res> &&response_handler)
{
    handle_func(std::move(request), timeout, std::move(response_handler));
}

}


template<typename Req, typename Res>
std::shared_ptr<request_handler<Req, Res>> make_func_request_handler(
    std::function<void(Req &&, std::chrono::nanoseconds, result_handler<Res> &&)> &&handle_func)
{
    return std::make_shared<func_request_handler_impl::func_request_handler<Req, Res>>(std::move(handle_func));
}

}

#endif /* NOSYNC__FUNC_REQUEST_HANDLER_IMPL_H */
