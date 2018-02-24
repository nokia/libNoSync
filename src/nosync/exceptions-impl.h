// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EXCEPTIONS_IMPL_H
#define NOSYNC__EXCEPTIONS_IMPL_H

#include <stdexcept>


namespace nosync
{

template<typename T>
T *get_nonnull_or_throw(T *ptr, const std::string &msg_for_null)
{
    if (ptr == nullptr) {
        throw std::runtime_error(msg_for_null);
    }

    return ptr;
}

}

#endif /* NOSYNC__EXCEPTIONS_IMPL_H */
