// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <type_traits>


namespace nosync
{

template<class InputIt, class UnaryOperation>
auto min_transformed_value(InputIt first, InputIt last, UnaryOperation unary_op) -> decltype(unary_op(*first))
{
    auto min_elem_iter = std::min_element(
        first, last,
        [&](const auto &lhs, const auto &rhs) {
            return unary_op(lhs) < unary_op(rhs);
        });
    return unary_op(*min_elem_iter);
}

}
