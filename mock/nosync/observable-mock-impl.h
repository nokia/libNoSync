// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__OBSERVABLE_MOCK_IMPL_H
#define NOSYNC__OBSERVABLE_MOCK_IMPL_H

#include <utility>


namespace nosync
{

std::unique_ptr<activity_handle> observable_no_rvref_wrapper::add_listener(std::function<void()> &&listener)
{
    return add_listener_impl(std::move(listener));
}

}

#endif /* NOSYNC__OBSERVABLE_MOCK_IMPL_H */
