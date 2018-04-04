// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__UTILS_H
#define NOSYNC__UTILS_H

#include <memory>


namespace nosync
{

template<class InputIt, class UnaryOperation>
auto min_transformed_value(InputIt first, InputIt last, UnaryOperation unary_op) -> decltype(unary_op(*first));

}

#include <nosync/utils-impl.h>

#endif /* NOSYNC__UTILS_H */
