// This file is part of libnosync library. See LICENSE file for license details.
#include <chrono>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/requests-queue.h>
#include <nosync/result-utils.h>
#include <nosync/type-utils.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::activity_handle_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using nosync::make_copy;
using nosync::make_error_result;
using nosync::make_ok_result;
using nosync::make_timeout_raw_error_result;
using nosync::requests_queue;
using nosync::result;
using std::errc;
using std::function;
using std::get;
using std::logic_error;
using std::make_error_code;
using std::make_unique;
using std::move;
using std::string;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::Return;
using testing::ReturnPointee;


TEST(NosyncRequestsQueue, CheckInitEmptyQueue)
{
    auto mock_evloop = make_unique<event_loop_mock>();

    auto req_queue = make_unique<requests_queue<unsigned, string>>(*mock_evloop);

    ASSERT_FALSE(req_queue->has_requests());
    ASSERT_THROW(req_queue->pull_next_request(), logic_error);
}


TEST(NosyncRequestsQueue, PullRequest)
{
    const auto current_time = ch::time_point<eclock>(ch::seconds(123456));
    constexpr auto read_timeout = 10s;

    auto mock_timeout_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_timeout_handle, disable()).Times(1);

    auto mock_evloop = make_unique<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    function<void()> saved_timeout_task;
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + read_timeout), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            saved_timeout_task = move(task);
            return move(mock_timeout_handle);
        }));

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    auto req_queue = make_unique<requests_queue<unsigned, string>>(*mock_evloop);

    const auto test_read_max_size = 10U;

    req_queue->push_request(make_copy(test_read_max_size), current_time + read_timeout, result_pusher);
    ASSERT_TRUE(req_queue->has_requests());

    ASSERT_EQ(results.size(), 0U);
    ASSERT_TRUE(saved_timeout_task);

    auto request = req_queue->pull_next_request();
    ASSERT_FALSE(req_queue->has_requests());

    ASSERT_EQ(get<unsigned>(request), test_read_max_size);
    ASSERT_EQ(get<ch::time_point<eclock>>(request), current_time + read_timeout);

    const auto test_result = make_ok_result("abc"s);

    get<function<void(result<string>)>>(request)(test_result);

    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), test_result);
}


TEST(NosyncRequestsQueue, RequestTimeout)
{
    auto current_time = ch::time_point<eclock>(ch::seconds(123456));
    constexpr auto read_timeout = 10s;

    auto mock_evloop = make_unique<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(ReturnPointee(&current_time));

    function<void()> saved_timeout_task;

    auto mock_timeout_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_timeout_handle, disable()).Times(0);

    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + read_timeout), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            saved_timeout_task = move(task);
            return move(mock_timeout_handle);
        }));

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    auto req_queue = make_unique<requests_queue<unsigned, string>>(*mock_evloop);

    req_queue->push_request(10U, current_time + read_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_TRUE(saved_timeout_task);

    current_time += read_timeout;
    saved_timeout_task();

    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_timeout_raw_error_result().as_result<string>());
}


TEST(NosyncRequestsQueue, CheckCancelOnDestroy)
{
    const auto current_time = ch::time_point<eclock>(ch::seconds(123456));
    constexpr auto read_timeout = 10s;

    auto mock_evloop = make_unique<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    function<void()> saved_timeout_task;

    auto mock_timeout_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_timeout_handle, disable()).WillOnce(Invoke(
        [&]() {
            saved_timeout_task = nullptr;
        }));
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + read_timeout), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            saved_timeout_task = move(task);
            return move(mock_timeout_handle);
        }));

    auto mock_cancel_handle = make_unique<activity_handle_mock>();

    function<void()> saved_cancel_task;
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            saved_cancel_task = move(task);
            return move(mock_cancel_handle);
        }));

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    auto req_queue = make_unique<requests_queue<unsigned, string>>(*mock_evloop);

    req_queue->push_request(10U, current_time + read_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_TRUE(saved_timeout_task);
    ASSERT_FALSE(saved_cancel_task);

    req_queue.reset();

    ASSERT_EQ(results.size(), 0U);
    ASSERT_FALSE(saved_timeout_task);
    ASSERT_TRUE(saved_cancel_task);

    saved_cancel_task();

    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_error_result<string>(make_error_code(errc::operation_canceled)));
}
