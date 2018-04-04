// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FUNCTION_UTILS_IMPL_H
#define NOSYNC__FUNCTION_UTILS_IMPL_H

#include <type_traits>


namespace nosync
{

namespace function_utils_impl
{

template<typename T, typename Op>
constexpr T reduce_impl(T init_value, Op)
{
   return init_value;
}


template<typename T, typename Op, typename ...TT>
constexpr T reduce_impl(T init_value, Op op, T head, TT ...values)
{
   return op(head, reduce_impl<T, Op>(init_value, op, values...));
}

}


template<typename ST, typename CCT, typename CT>
void copy_from_supplier_to_consumer(const ST &supplier, const CCT &continue_condition, const CT &consumer)
{
    using value_type = decltype(supplier());
    while (true) {
        const value_type &value = supplier();
        if (!continue_condition(value)) {
            break;
        }

        consumer(value);
    }
}


template<typename ST, typename CT>
void copy_from_supplier_to_consumer(const ST &supplier, const CT &consumer)
{
    using value_type = decltype(supplier());
    std::function<bool(const value_type &)> continue_condition = [empty_value = value_type()](const value_type &value) {
        return value != empty_value;
    };

    copy_from_supplier_to_consumer(supplier, continue_condition, consumer);
}


template<typename T, typename Op, typename ...TT>
constexpr T reduce(T init_value, Op op, TT ...values)
{
   return function_utils_impl::reduce_impl<T, Op>(init_value, op, values...);
}

}

#endif /* NOSYNC__FUNCTION_UTILS_IMPL_H */
