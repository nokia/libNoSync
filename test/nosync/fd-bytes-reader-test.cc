// This file is part of libnosync library. See LICENSE file for license details.
#include <array>
#include <experimental/string_view>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/event-loop.h>
#include <nosync/exceptions.h>
#include <nosync/fd-bytes-reader.h>
#include <nosync/fd-watching-event-loop-mock.h>
#include <nosync/owned-fd.h>
#include <nosync/ppoll-based-event-loop.h>
#include <nosync/result-utils.h>
#include <nosync/result.h>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::activity_handle_mock;
using nosync::eclock;
using nosync::fd_watch_mode;
using nosync::fd_watching_event_loop_mock;
using nosync::make_timeout_raw_error_result;
using nosync::make_ok_result;
using nosync::make_fd_bytes_reader;
using nosync::make_ppoll_based_event_loop;
using nosync::owned_fd;
using nosync::result;
using nosync::throw_system_error_from_errno;
using std::array;
using std::experimental::string_view;
using std::function;
using std::make_shared;
using std::make_unique;
using std::move;
using std::string;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::ReturnPointee;


namespace
{

constexpr auto very_long_timeout = 1h;


array<owned_fd, 2> create_nonblocking_pipe()
{
    int pipe_fds[2];
    int pipe_retval = ::pipe2(pipe_fds, O_CLOEXEC | O_NONBLOCK);
    if (pipe_retval < 0) {
        throw_system_error_from_errno("failed to create pipe");
    }

    return {owned_fd(pipe_fds[0]), owned_fd(pipe_fds[1])};
}


void write_to_fd_fully(int fd, string_view data)
{
    size_t data_offset = 0;
    while (data_offset < data.size()) {
        ssize_t write_result;
        do {
            write_result = write(fd, &data[data_offset], data.size() - data_offset);
        } while (write_result == -1 && errno == EINTR);

        ASSERT_GE(write_result, 0);

        data_offset += write_result;
    }
}

}


TEST(NosyncFdBytesReader, TestTimeoutNew)
{
    constexpr auto read_timeout = 500ns;
    const auto current_time = ch::time_point<eclock>(123s);

    auto pipe_fds = create_nonblocking_pipe();

    function<void()> saved_sock_watch_notify_func;
    auto mock_sock_watch_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_sock_watch_handle, disable()).WillOnce(Invoke(
        [&saved_sock_watch_notify_func]() {
            saved_sock_watch_notify_func = nullptr;
        }));

    auto mock_timeout_task_handle = make_unique<activity_handle_mock>();
    auto mock_evloop = make_shared<fd_watching_event_loop_mock>();
    EXPECT_CALL(*mock_evloop, add_watch_impl(Eq(*pipe_fds[0]), Eq(fd_watch_mode::input), _)).WillOnce(Invoke(
        [&](auto, auto, auto notify_func) {
            saved_sock_watch_notify_func = move(notify_func);
            return move(mock_sock_watch_handle);
        }));
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + read_timeout), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            task();
            return move(mock_timeout_task_handle);
        }));
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(ReturnPointee(&current_time));

//    auto test_evloop = make_ppoll_based_event_loop();

    vector<result<string>> saved_results;
    auto reader = make_fd_bytes_reader(*mock_evloop, move(pipe_fds[0]));
    reader->read_some_bytes(
        10U, read_timeout,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });

//    test_evloop->run_iterations();

    ASSERT_EQ(saved_results.size(), 1U);
    ASSERT_EQ(saved_results[0], make_timeout_raw_error_result().as_result<string>());
}


TEST(NosyncFdBytesReader, TestTimeout)
{
    auto test_evloop = make_ppoll_based_event_loop();
    auto pipe_fds = create_nonblocking_pipe();

    vector<result<string>> saved_results;
    auto reader = make_fd_bytes_reader(*test_evloop, move(pipe_fds[0]));
    reader->read_some_bytes(
        10U, 1ns,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });

    test_evloop->run_iterations();

    ASSERT_EQ(saved_results.size(), 1U);
    ASSERT_EQ(saved_results[0], make_timeout_raw_error_result().as_result<string>());
}


TEST(NosyncFdBytesReader, TestEof)
{
    auto test_evloop = make_ppoll_based_event_loop();
    auto pipe_fds = create_nonblocking_pipe();

    pipe_fds[1] = owned_fd();

    vector<result<string>> saved_results;
    auto reader = make_fd_bytes_reader(*test_evloop, move(pipe_fds[0]));
    reader->read_some_bytes(
        10U, very_long_timeout,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });

    test_evloop->run_iterations();

    ASSERT_EQ(saved_results.size(), 1U);
    ASSERT_EQ(saved_results[0], make_ok_result(""s));
}


TEST(NosyncFdBytesReader, TestMultipleReads)
{
    const auto test_data = "abcdef"s;

    auto test_evloop = make_ppoll_based_event_loop();
    auto pipe_fds = create_nonblocking_pipe();

    write_to_fd_fully(*pipe_fds[1], test_data);
    pipe_fds[1] = owned_fd();

    vector<result<string>> saved_results;
    auto reader = make_fd_bytes_reader(*test_evloop, move(pipe_fds[0]));
    reader->read_some_bytes(
        2U, very_long_timeout,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });
    reader->read_some_bytes(
        3U, very_long_timeout,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });
    reader->read_some_bytes(
        10U, very_long_timeout,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });
    reader->read_some_bytes(
        10U, very_long_timeout,
        [&saved_results](auto result) {
            saved_results.push_back(move(result));
        });

    test_evloop->run_iterations();

    ASSERT_EQ(saved_results.size(), 4U);
    ASSERT_EQ(saved_results[0], make_ok_result("ab"s));
    ASSERT_EQ(saved_results[1], make_ok_result("cde"s));
    ASSERT_EQ(saved_results[2], make_ok_result("f"s));
    ASSERT_EQ(saved_results[3], make_ok_result(""s));
}
