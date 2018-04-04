// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_HANDLER_H
#define NOSYNC__RESULT_HANDLER_H

#include <functional>
#include <nosync/result.h>


namespace nosync
{

/*!
Alias template for std::function<> which accepts result<> parameter.

This alias is to be used mostly as a parameter type in functions which accept
"result handlers". Result handler is a function which is to be called
asynchonously to return the result of an operation.

Note: generally I'm against using aliases/typedefs for complex types just to
"save some characters" as this hides real types and therefore can make the code
less readable. However, contrary to type aliases in "user code", this one is to
be widely used idiom and therefore it should not reduce code readability.
*/
template<typename T>
using result_handler = std::function<void(result<T>)>;


template<typename InType, typename OutType, typename F>
result_handler<InType> transform_result_handler(
    result_handler<OutType> &&base_res_handler, F &&trans_func);

template<typename InType, typename OutType>
result_handler<InType> transform_result_handler(
    result_handler<OutType> &&base_res_handler, std::function<result<OutType>(InType)> &&trans_func);

template<typename InType, typename OutType, typename F>
result_handler<InType> merge_result_handler(
   result_handler<OutType> &&base_res_handler, F &&handle_func);

template<typename InType, typename OutType>
result_handler<InType> merge_result_handler(
    result_handler<OutType> &&base_res_handler, std::function<void(InType, result_handler<OutType>)> &&handle_func);

}

#include <nosync/result-handler-impl.h>

#endif /* NOSYNC__RESULT_HANDLER_H */
