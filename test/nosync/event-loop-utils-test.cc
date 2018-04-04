// This file is part of libnosync library. See LICENSE file for license details.
#include <chrono>
#include <experimental/array>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/event-loop-utils.h>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using nosync::activity_handle_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using std::experimental::make_array;
using std::make_shared;
using std::make_unique;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::ReturnPointee;


namespace
{

constexpr auto test_start_time = ch::time_point<eclock>(ch::seconds(123456));
constexpr auto test_time_points = make_array(
    test_start_time + 0s,
    test_start_time + 2s,
    test_start_time + 10s);

}


TEST(NosyncEventLoopUtils, TestInvokeLater)
{
    auto mock_evloop = make_shared<event_loop_mock>();

    auto mock_handle = make_unique<activity_handle_mock>();
    auto mock_handle_ptr = mock_handle.get();

    const auto current_time = ch::time_point<eclock>(ch::seconds(123456));

    auto task_call_counter = 0U;

    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(ReturnPointee(&current_time));
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            task();
            return move(mock_handle);
        }));

    auto handle = invoke_later(
        *mock_evloop,
        [&]() {
            ++task_call_counter;
        });
    ASSERT_EQ(handle.get(), mock_handle_ptr);
    ASSERT_EQ(task_call_counter, 1U);
}


TEST(NosyncEventLoopUtils, TestInvokeWithEtimeDelay)
{
    auto mock_evloop = make_shared<event_loop_mock>();

    auto mock_handle = make_unique<activity_handle_mock>();
    auto mock_handle_ptr = mock_handle.get();

    const auto current_time = ch::time_point<eclock>(ch::seconds(234567));
    constexpr auto invoke_delay = 34ms;

    auto task_call_counter = 0U;

    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(ReturnPointee(&current_time));
    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + invoke_delay), _)).WillOnce(Invoke(
        [&](auto, auto task) {
            task();
            return move(mock_handle);
        }));

    auto handle = invoke_with_etime_delay(
        *mock_evloop, invoke_delay,
        [&]() {
            ++task_call_counter;
        });
    ASSERT_EQ(handle.get(), mock_handle_ptr);
    ASSERT_EQ(task_call_counter, 1U);
}
