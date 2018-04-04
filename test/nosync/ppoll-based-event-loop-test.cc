// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <chrono>
#include <fcntl.h>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/ppoll-based-event-loop.h>
#include <nosync/type-utils.h>
#include <system_error>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::activity_handle;
using nosync::eclock;
using nosync::fd_watch_mode;
using nosync::make_copy;
using nosync::make_ppoll_based_event_loop;
using std::errc;
using std::function;
using std::make_error_code;
using std::unique_ptr;
using std::vector;


namespace
{

constexpr auto small_time_increment = 1ns;

}


TEST(NosyncPpollBasedEventLoop, TestSimpleTask)
{
    auto evloop = make_ppoll_based_event_loop();

    auto counter = 0U;

    evloop->invoke_at(
        evloop->get_etime() + small_time_increment,
        [&]() {
            ++counter;
        });

    ASSERT_EQ(counter, 0U);
    ASSERT_FALSE(evloop->run_iterations());
    ASSERT_EQ(counter, 1U);
}


TEST(NosyncPpollBasedEventLoop, TestTasksWithTimeIncrements)
{
    constexpr auto task_repetitions = 5U;

    auto evloop = make_ppoll_based_event_loop();

    const auto start_time = evloop->get_etime();
    vector<ch::time_point<eclock>> task_times;

    function<void()> task;
    task = [&]() {
        task_times.push_back(evloop->get_etime());
        if (task_times.size() < task_repetitions) {
            evloop->invoke_at(evloop->get_etime() + small_time_increment, make_copy(task));
        }
    };

    evloop->invoke_at(evloop->get_etime() + small_time_increment, make_copy(task));
    ASSERT_FALSE(evloop->run_iterations());

    ASSERT_EQ(task_times.size(), task_repetitions);

    auto prev_task_time = start_time;
    for (auto task_time : task_times) {
        ASSERT_GT(task_time, prev_task_time);
        prev_task_time = task_time;
    }
}


TEST(NosyncPpollBasedEventLoop, TestTasksNoTimeIncrements)
{
    constexpr auto task_repetitions = 5U;

    auto evloop = make_ppoll_based_event_loop();

    vector<ch::time_point<eclock>> task_times;

    for (auto i = 0U; i < task_repetitions; ++i) {
        evloop->invoke_at(
            evloop->get_etime() + small_time_increment,
            [&]() {
                task_times.push_back(evloop->get_etime());
            });
    }

    ASSERT_FALSE(evloop->run_iterations());

    ASSERT_EQ(task_times.size(), task_repetitions);

    for (const auto task_time : task_times) {
        ASSERT_EQ(task_time, task_times.front());
    }
}


TEST(NosyncPpollBasedEventLoop, TestFdWatch)
{
    auto evloop = make_ppoll_based_event_loop();

    int pipe_fds_a[2];
    ASSERT_EQ(::pipe2(pipe_fds_a, O_CLOEXEC), 0);
    unique_ptr<activity_handle> activity_handle_a;

    int pipe_fds_b[2];
    ASSERT_EQ(::pipe2(pipe_fds_b, O_CLOEXEC), 0);
    unique_ptr<activity_handle> activity_handle_b;

    auto exec_trace = ""s;

    evloop->invoke_at(
        evloop->get_etime() + small_time_increment,
        [&]() {
            exec_trace.push_back('S');
            activity_handle_a = evloop->add_watch(
                pipe_fds_a[0], fd_watch_mode::input,
                [&]() {
                    exec_trace.push_back('R');
                    activity_handle_a->disable();
                });
            activity_handle_b = evloop->add_watch(
                pipe_fds_b[0], fd_watch_mode::input,
                [&]() {
                    exec_trace.push_back('r');
                    activity_handle_b->disable();
                });
            evloop->invoke_at(
                evloop->get_etime() + small_time_increment,
                [&]() {
                    exec_trace.push_back('W');
                    ::write(pipe_fds_a[1], "", 1);
                });
        });
    evloop->invoke_at(
        evloop->get_etime(),
        [&]() {
            exec_trace.push_back('w');
            ::write(pipe_fds_b[1], "", 1);
        });

    ASSERT_EQ(exec_trace, ""s);
    ASSERT_FALSE(evloop->run_iterations());
    ASSERT_EQ(exec_trace, "wSrWR"s);
}


TEST(NosyncPpollBasedEventLoop, TestQuitBetweenTasks)
{
    auto evloop = make_ppoll_based_event_loop();

    auto exec_trace = ""s;

    unique_ptr<activity_handle> activity_handle;

    evloop->invoke_at(
        evloop->get_etime() + small_time_increment,
        [&]() {
            exec_trace.push_back('1');
            evloop->quit();
        });
    evloop->invoke_at(
        evloop->get_etime() + small_time_increment * 2,
        [&]() {
            exec_trace.push_back('2');
        });

    ASSERT_EQ(exec_trace, ""s);
    ASSERT_EQ(evloop->run_iterations(), make_error_code(errc::interrupted));
    ASSERT_EQ(exec_trace, "1"s);
}


TEST(NosyncPpollBasedEventLoop, TestQuitBetweenWatches)
{
    auto evloop = make_ppoll_based_event_loop();

    int pipe_fds[2];
    ASSERT_EQ(::pipe2(pipe_fds, O_CLOEXEC), 0);

    auto exec_trace = ""s;

    evloop->add_watch(
        pipe_fds[0], fd_watch_mode::input,
        [&]() {
            exec_trace.push_back('1');
            evloop->quit();
        });
    evloop->add_watch(
        pipe_fds[0], fd_watch_mode::input,
        [&]() {
            exec_trace.push_back('2');
        });

    ::write(pipe_fds[1], "", 1);

    ASSERT_EQ(exec_trace, ""s);
    ASSERT_EQ(evloop->run_iterations(), make_error_code(errc::interrupted));
    ASSERT_EQ(exec_trace, "1"s);
}
