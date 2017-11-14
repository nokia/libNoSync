// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EVENT_LOOP_MOCK_H
#define NOSYNC__EVENT_LOOP_MOCK_H

#include <gmock/gmock.h>
#include <nosync/event-loop.h>


namespace nosync
{

class event_loop_no_rvref_wrapper : public event_loop
{
public:
    event_loop_no_rvref_wrapper() = default;

    std::unique_ptr<activity_handle> invoke_at(std::chrono::time_point<eclock> time, std::function<void()> &&task) override final;

    virtual std::unique_ptr<activity_handle> invoke_at_impl(std::chrono::time_point<eclock> time, std::function<void()> task) = 0;
};


class event_loop_mock : public event_loop_no_rvref_wrapper
{
public:
    event_loop_mock() = default;

    MOCK_METHOD2(invoke_at_impl, std::unique_ptr<activity_handle>(std::chrono::time_point<eclock>, std::function<void()>));
    MOCK_CONST_METHOD0(get_etime, std::chrono::time_point<eclock>());
};

}

#include <nosync/event-loop-mock-impl.h>

#endif /* NOSYNC__EVENT_LOOP_MOCK_H */
