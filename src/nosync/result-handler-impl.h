// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_HANDLER_IMPL_H
#define NOSYNC__RESULT_HANDLER_IMPL_H

#include <nosync/raw-error-result.h>
#include <type_traits>


namespace nosync
{

namespace result_handler_impl
{

template<typename InType, typename OutType, typename F>
struct result_handler_merger
{
    static_assert(std::is_assignable<std::function<void(InType, result_handler<OutType>)>, F>::value, "parameter must be proper function-like type");
    result_handler<InType> operator()(result_handler<OutType> &&base_res_handler, F &&handle_func) const;
};


template<typename OutType, typename F>
struct result_handler_merger<void, OutType, F>
{
    static_assert(std::is_assignable<std::function<void(result_handler<OutType>)>, F>::value, "parameter must be proper function-like type");
    result_handler<void> operator()(result_handler<OutType> &&base_res_handler, F &&handle_func) const;
};


template<typename InType, typename OutType, typename F>
struct result_handler_transformer
{
    static_assert(std::is_assignable<std::function<result<OutType>(InType)>, F>::value, "parameter must be proper function-like type");
    result_handler<InType> operator()(result_handler<OutType> &&base_res_handler, F &&trans_func) const;
};


template<typename OutType, typename F>
struct result_handler_transformer<void, OutType, F>
{
    static_assert(std::is_assignable<std::function<result<OutType>()>, F>::value, "parameter must be proper function-like type");
    result_handler<void> operator()(result_handler<OutType> &&base_res_handler, F &&trans_func) const;
};


template<typename InType, typename OutType, typename F>
result_handler<InType> result_handler_merger<InType, OutType, F>::operator()(
    result_handler<OutType> &&base_res_handler, F &&handle_func) const
{
    return [base_res_handler = std::move(base_res_handler), handle_func = std::forward<F>(handle_func)](result<InType> res) mutable {
        if (res.is_ok()) {
            handle_func(std::move(res.get_value()), std::move(base_res_handler));
        } else {
            base_res_handler(raw_error_result(res));
        }
    };
}


template<typename OutType, typename F>
result_handler<void> result_handler_merger<void, OutType, F>::operator()(
    result_handler<OutType> &&base_res_handler, F &&handle_func) const
{
    return [base_res_handler = std::move(base_res_handler), handle_func = std::forward<F>(handle_func)](result<void> res) mutable {
        if (res.is_ok()) {
            handle_func(std::move(base_res_handler));
        } else {
            base_res_handler(raw_error_result(res));
        }
    };
}


template<typename InType, typename OutType, typename F>
result_handler<InType> result_handler_transformer<InType, OutType, F>::operator()(
    result_handler<OutType> &&base_res_handler, F &&trans_func) const
{
    return [base_res_handler = std::move(base_res_handler), trans_func = std::forward<F>(trans_func)](result<InType> res) mutable {
        base_res_handler(
            res.is_ok()
                ? trans_func(std::move(res.get_value()))
                : raw_error_result(res));
    };
}


template<typename OutType, typename F>
result_handler<void> result_handler_transformer<void, OutType, F>::operator()(
    result_handler<OutType> &&base_res_handler, F &&trans_func) const
{
    return [base_res_handler = std::move(base_res_handler), trans_func = std::forward<F>(trans_func)](result<void> res) mutable {
        base_res_handler(
            res.is_ok()
                ? trans_func()
                : raw_error_result(res));
    };
}

}


template<typename InType, typename OutType, typename F>
result_handler<InType> transform_result_handler(result_handler<OutType> &&base_res_handler, F &&trans_func)
{
    return result_handler_impl::result_handler_transformer<InType, OutType, F>()(move(base_res_handler), std::forward<F>(trans_func));
}


template<typename InType, typename OutType, typename F>
result_handler<InType> merge_result_handler(result_handler<OutType> &&base_res_handler, F &&handle_func)
{
    return result_handler_impl::result_handler_merger<InType, OutType, F>()(std::move(base_res_handler), std::forward<F>(handle_func));
}

}

#endif /* NOSYNC__RESULT_HANDLER_IMPL_H */
