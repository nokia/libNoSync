// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EVENT_LOOP_MOCK_IMPL_H
#define NOSYNC__EVENT_LOOP_MOCK_IMPL_H

#include <utility>


namespace nosync
{

inline std::unique_ptr<activity_handle> event_loop_no_rvref_wrapper::invoke_at(std::chrono::time_point<eclock> time, std::function<void()> &&task)
{
    return invoke_at_impl(time, std::move(task));
}

}

#endif /* NOSYNC__EVENT_LOOP_MOCK_IMPL_H */
