// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_WATCHING_EVENT_LOOP_MOCK_H
#define NOSYNC__FD_WATCHING_EVENT_LOOP_MOCK_H

#include <gmock/gmock.h>
#include <nosync/fd-watching-event-loop.h>


namespace nosync
{

class fd_watching_event_loop_no_rvref_wrapper : public fd_watching_event_loop
{
public:
    fd_watching_event_loop_no_rvref_wrapper() = default;

    std::unique_ptr<activity_handle> invoke_at(std::chrono::time_point<eclock> time, std::function<void()> &&task) override final;
    std::unique_ptr<activity_handle> add_watch(int fd, fd_watch_mode mode, std::function<void()> &&notify_func) override final;

    virtual std::unique_ptr<activity_handle> invoke_at_impl(std::chrono::time_point<eclock> time, std::function<void()> task) = 0;
    virtual std::unique_ptr<activity_handle> add_watch_impl(int fd, fd_watch_mode mode, std::function<void()> notify_func) = 0;
};


class fd_watching_event_loop_mock : public fd_watching_event_loop_no_rvref_wrapper
{
public:
    fd_watching_event_loop_mock() = default;

    MOCK_METHOD2(invoke_at_impl, std::unique_ptr<activity_handle>(std::chrono::time_point<eclock>, std::function<void()>));
    MOCK_CONST_METHOD0(get_etime, std::chrono::time_point<eclock>());

    MOCK_METHOD3(add_watch_impl, std::unique_ptr<activity_handle>(int, fd_watch_mode, std::function<void()>));
};

}

#include <nosync/fd-watching-event-loop-mock-impl.h>

#endif /* NOSYNC__FD_WATCHING_EVENT_LOOP_MOCK_H */
