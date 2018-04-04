// This file is part of libnosync library. See LICENSE file for license details.
#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/request-handler-mock.h>
#include <nosync/transforming-request-handler.h>
#include <nosync/type-utils.h>
#include <system_error>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using nosync::activity_handle_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using nosync::make_copy;
using nosync::make_error_result;
using nosync::make_ok_result;
using nosync::make_transforming_request_handler;
using nosync::request_handler_mock;
using nosync::result;
using std::errc;
using std::function;
using std::make_error_code;
using std::make_shared;
using std::make_unique;
using std::move;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::Return;


namespace
{

constexpr auto input_threshold = 15;

const auto test_error_code = make_error_code(errc::bad_message);


function<result<bool>(int)> make_test_request_transformer()
{
    return [](int input) {
        return input != input_threshold
            ? make_ok_result(input >= input_threshold)
            : make_error_result<bool>(test_error_code);
    };
}

}


TEST(NosyncTransformingRequestHandler, TestTransformOk)
{
    const auto current_time = ch::time_point<eclock>(123s);
    constexpr auto test_timeout = 3ns;

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    vector<function<void(result<char>)>> saved_res_handlers;

    auto mock_handler = make_shared<request_handler_mock<bool, char>>();
    EXPECT_CALL(*mock_handler, handle_request_impl(Eq(false), Eq(test_timeout), _)).WillOnce(Invoke(
        [&saved_res_handlers](auto, auto, auto res_handler) {
            saved_res_handlers.push_back(move(res_handler));
        }));

    auto trans_handler = make_transforming_request_handler<int, bool, char>(
        *mock_evloop, mock_handler, make_test_request_transformer());

    vector<result<char>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    trans_handler->handle_request(input_threshold - 1, test_timeout, result_pusher);

    const auto test_result = make_ok_result('X');

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_res_handlers.size(), 1U);
    saved_res_handlers.front()(test_result);
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), test_result);
}


TEST(NosyncTransformingRequestHandler, TestTransformError)
{
    const auto current_time = ch::time_point<eclock>(123s);
    constexpr auto test_timeout = 3ns;

    auto mock_evloop = make_shared<event_loop_mock>();

    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    auto mock_task_handle = make_unique<activity_handle_mock>();
    function<void()> saved_task;
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time), _)).WillOnce(Invoke(
        [&saved_task, &mock_task_handle](auto, auto task) {
            saved_task = move(task);
            return move(mock_task_handle);
        }));

    auto mock_handler = make_shared<request_handler_mock<bool, char>>();

    auto trans_handler = make_transforming_request_handler<int, bool, char>(
        *mock_evloop, mock_handler, make_test_request_transformer());

    vector<result<char>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    trans_handler->handle_request(make_copy(input_threshold), test_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_TRUE(saved_task);
    saved_task();
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_error_result<char>(test_error_code));
}
