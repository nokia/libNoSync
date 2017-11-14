// This file is part of libnosync library. See LICENSE file for license details.
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>
#include <nosync/bytes-reader-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/result.h>
#include <nosync/size-limited-bytes-reader.h>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::activity_handle_mock;
using nosync::bytes_reader_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using nosync::make_error_result;
using nosync::make_ok_result;
using nosync::make_size_limited_bytes_reader;
using nosync::result;
using std::errc;
using std::function;
using std::get;
using std::make_error_code;
using std::make_shared;
using std::make_unique;
using std::move;
using std::string;
using std::tuple;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Gt;
using testing::Invoke;
using testing::Return;


TEST(NosyncSizeLimitedBytesReader, ZeroRead)
{
    constexpr auto read_timeout = 3ns;
    const auto current_time = ch::time_point<eclock>(123s);

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    vector<function<void(result<string>)>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(0U), _, _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto, auto, auto result_handler) {
            saved_result_handlers.push_back(move(result_handler));
        }));

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    auto test_reader = make_size_limited_bytes_reader(*mock_evloop, 15U, mock_reader);

    test_reader->read_some_bytes(0U, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    saved_result_handlers.front()(make_ok_result(""s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result(""s));
    saved_result_handlers.clear();
    results.clear();
}


TEST(NosyncSizeLimitedBytesReader, SequentialReads)
{
    constexpr auto read_timeout = 3ns;
    const auto current_time = ch::time_point<eclock>(123s);

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    vector<tuple<size_t, function<void(result<string>)>>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(0U), _, _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Gt(0U), Eq(read_timeout), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    auto test_reader = make_size_limited_bytes_reader(*mock_evloop, 15U, mock_reader);

    const auto test_chunk_1 = "1x2x3x4x"s;
    test_reader->read_some_bytes(100U, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 15U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result(test_chunk_1));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result(test_chunk_1));
    saved_result_handlers.clear();
    results.clear();

    const auto test_chunk_2 = "1y2y3y4"s;
    test_reader->read_some_bytes(100U, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 7U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result(test_chunk_2));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result(test_chunk_2));
    saved_result_handlers.clear();
    results.clear();

    test_reader->read_some_bytes(100U, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 0U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result(""s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_error_result<string>(make_error_code(errc::file_too_large)));
    saved_result_handlers.clear();
    results.clear();
}


TEST(NosyncSizeLimitedBytesReader, ConcurrentReads)
{
    constexpr auto read_timeout = 3ns;
    const auto current_time = ch::time_point<eclock>(123s);

    auto mock_evloop = make_shared<event_loop_mock>();

    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(current_time));

    auto mock_timeout_task_handle = make_unique<activity_handle_mock>();
    EXPECT_CALL(*mock_timeout_task_handle, disable()).Times(1);

    EXPECT_CALL(*mock_evloop, invoke_at_impl(Eq(current_time + read_timeout), _)).WillOnce(Invoke(
        [&mock_timeout_task_handle](auto, auto) {
            return move(mock_timeout_task_handle);
        }));

    auto mock_reader = make_shared<bytes_reader_mock>();

    vector<tuple<size_t, function<void(result<string>)>>> saved_result_handlers;
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Gt(0U), Eq(read_timeout), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));

    auto test_reader = make_size_limited_bytes_reader(*mock_evloop, 15U, mock_reader);

    vector<result<string>> results_a;
    test_reader->read_some_bytes(
        100U, read_timeout,
        [&results_a](auto result) {
            results_a.push_back(move(result));
        });

    vector<result<string>> results_b;
    test_reader->read_some_bytes(
        100U, read_timeout,
        [&results_b](auto result) {
            results_b.push_back(move(result));
        });

    ASSERT_EQ(results_a.size(), 0U);
    ASSERT_EQ(results_b.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.back()), 15U);

    const auto test_chunk_1 = "1x2x3x4x"s;
    get<function<void(result<string>)>>(saved_result_handlers.back())(make_ok_result(test_chunk_1));

    ASSERT_EQ(results_a.size(), 1U);
    ASSERT_EQ(results_a.back(), make_ok_result(test_chunk_1));
    ASSERT_EQ(results_b.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 2U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.back()), 7U);

    const auto test_chunk_2 = "1y2y3y4"s;
    get<function<void(result<string>)>>(saved_result_handlers.back())(make_ok_result(test_chunk_2));

    ASSERT_EQ(results_a.size(), 1U);
    ASSERT_EQ(results_a.back(), make_ok_result(test_chunk_1));
    ASSERT_EQ(results_b.size(), 1U);
    ASSERT_EQ(results_b.back(), make_ok_result(test_chunk_2));
    ASSERT_EQ(saved_result_handlers.size(), 2U);
}
