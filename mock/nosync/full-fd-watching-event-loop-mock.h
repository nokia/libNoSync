// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FULL_FD_WATCHING_EVENT_LOOP_MOCK_H
#define NOSYNC__FULL_FD_WATCHING_EVENT_LOOP_MOCK_H

#include <gmock/gmock.h>
#include <nosync/full-fd-watching-event-loop.h>


namespace nosync
{

class full_fd_watching_event_loop_no_rvref_wrapper : public full_fd_watching_event_loop
{
public:
    full_fd_watching_event_loop_no_rvref_wrapper() = default;

    std::unique_ptr<activity_handle> invoke_at(std::chrono::time_point<eclock> time, std::function<void()> &&task) override final;
    std::unique_ptr<activity_handle> add_watch(int fd, fd_watch_mode mode, std::function<void()> &&notify_func) override final;

    virtual std::unique_ptr<activity_handle> invoke_at_impl(std::chrono::time_point<eclock> time, std::function<void()> task) = 0;
    virtual std::unique_ptr<activity_handle> add_watch_impl(int fd, fd_watch_mode mode, std::function<void()> notify_func) = 0;
};


class full_fd_watching_event_loop_mock : public full_fd_watching_event_loop_no_rvref_wrapper
{
public:
    full_fd_watching_event_loop_mock() = default;

    MOCK_METHOD2(invoke_at_impl, std::unique_ptr<activity_handle>(std::chrono::time_point<eclock>, std::function<void()>));
    MOCK_CONST_METHOD0(get_etime, std::chrono::time_point<eclock>());

    MOCK_METHOD3(add_watch_impl, std::unique_ptr<activity_handle>(int, fd_watch_mode, std::function<void()>));

    MOCK_METHOD0(run_iterations, std::error_code());
    MOCK_METHOD0(quit, void());
};

}

#include <nosync/full-fd-watching-event-loop-mock-impl.h>

#endif /* NOSYNC__FULL_FD_WATCHING_EVENT_LOOP_MOCK_H */
