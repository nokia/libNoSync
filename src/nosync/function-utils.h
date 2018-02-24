// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FUNCTION_UTILS_H
#define NOSYNC__FUNCTION_UTILS_H

#include <functional>
#include <string>


namespace nosync
{

std::function<std::string(std::size_t)> make_buffering_bytes_supplier(std::function<std::string()> &&base_supplier);

template<typename ST, typename CCT, typename CT>
void copy_from_supplier_to_consumer(const ST &supplier, const CCT &continue_condition, const CT &consumer);

template<typename ST, typename CT>
void copy_from_supplier_to_consumer(const ST &supplier, const CT &consumer);

template<typename T, typename Op, typename ...TT>
constexpr T reduce(T init_value, Op op, TT ...values);

}

#include <nosync/function-utils-impl.h>

#endif /* NOSYNC__FUNCTION_UTILS_H */
