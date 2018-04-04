// This file is part of libnosync library. See LICENSE file for license details.
#include <chrono>
#include <experimental/array>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/prioritizing-request-handler.h>
#include <nosync/request-handler-mock.h>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using nosync::activity_handle_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using nosync::make_ok_result;
using nosync::make_prioritizing_request_handler;
using nosync::request_handler;
using nosync::request_handler_mock;
using nosync::result;
using std::function;
using std::experimental::make_array;
using std::make_shared;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::vector;
using testing::_;
using testing::Eq;
using testing::InSequence;
using testing::Invoke;
using testing::Return;


TEST(NosyncPrioritizingRequestHandler, DirectForwarding)
{
    const auto current_time = ch::time_point<eclock>(123s);
    constexpr auto test_timeout = 3ns;

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    vector<function<void(result<char>)>> saved_res_handlers;

    auto mock_handler = make_shared<request_handler_mock<int, char>>();
    EXPECT_CALL(*mock_handler, handle_request_impl(Eq(123), Eq(3ns), _)).WillOnce(Invoke(
        [&saved_res_handlers](auto, auto, auto res_handler) {
            saved_res_handlers.push_back(move(res_handler));
        }));

    shared_ptr<request_handler<int, char>> base_handler = mock_handler;
    auto prio_handlers = make_prioritizing_request_handler(*mock_evloop, move(base_handler));

    vector<result<char>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    prio_handlers[1]->handle_request(123, 3ns, result_pusher);

    const auto test_result_1 = make_ok_result('X');
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_res_handlers.size(), 1U);
    saved_res_handlers.front()(test_result_1);
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), test_result_1);
}


TEST(NosyncPrioritizingRequestHandler, DelayedLowPrio)
{
    const auto current_time = ch::time_point<eclock>(123s);
    constexpr auto test_timeout = 3ns;

    auto mock_evloop = make_shared<event_loop_mock>();

    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + test_timeout), _)).Times(2).WillRepeatedly(Invoke(
        [](auto, auto) {
            auto mock_timeout_task_handle = make_unique<activity_handle_mock>();
            EXPECT_CALL(*mock_timeout_task_handle, disable()).Times(1);
            return move(mock_timeout_task_handle);
        }));

    vector<function<void(result<char>)>> saved_res_handlers;
    auto saved_res_handlers_pusher = [&saved_res_handlers](auto, auto, auto res_handler) {
        saved_res_handlers.push_back(move(res_handler));
    };

    auto mock_handler = make_shared<request_handler_mock<int, char>>();

    {
        InSequence seq;
        EXPECT_CALL(*mock_handler, handle_request_impl(Eq(123), Eq(test_timeout), _)).WillOnce(Invoke(saved_res_handlers_pusher));
        EXPECT_CALL(*mock_handler, handle_request_impl(Eq(789), Eq(test_timeout), _)).WillOnce(Invoke(saved_res_handlers_pusher));
        EXPECT_CALL(*mock_handler, handle_request_impl(Eq(456), Eq(test_timeout), _)).WillOnce(Invoke(saved_res_handlers_pusher));
    }

    shared_ptr<request_handler<int, char>> base_handler = mock_handler;
    auto prio_handlers = make_prioritizing_request_handler(*mock_evloop, move(base_handler));

    vector<result<char>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    prio_handlers[1]->handle_request(123, test_timeout, result_pusher);
    prio_handlers[1]->handle_request(456, test_timeout, result_pusher);
    prio_handlers[0]->handle_request(789, test_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);

    const auto res_values = make_array('X', 'Y', 'Z');
    for (unsigned i = 0; i < res_values.size(); ++i) {
        const auto test_result = make_ok_result(res_values[i]);
        ASSERT_EQ(saved_res_handlers.size(), i + 1);
        saved_res_handlers.back()(test_result);
        ASSERT_EQ(results.size(), i + 1);
        ASSERT_EQ(results.back(), test_result);
    }

    ASSERT_EQ(saved_res_handlers.size(), 3U);
}
