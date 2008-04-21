// This file is part of libnosync library. See LICENSE file for license details.
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-reader-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/result-utils.h>
#include <nosync/result.h>
#include <nosync/time-limited-bytes-reader.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_reader_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using nosync::make_ok_result;
using nosync::make_time_limited_bytes_reader;
using nosync::make_timeout_raw_error_result;
using nosync::result;
using std::function;
using std::get;
using std::make_shared;
using std::move;
using std::string;
using std::tuple;
using std::vector;
using testing::_;
using testing::Invoke;
using testing::ReturnPointee;


TEST(NosyncTimeLimitedBytesReader, TestSimple)
{
    ch::time_point<eclock> current_time(123s);

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(ReturnPointee(&current_time));

    vector<tuple<size_t, ch::nanoseconds, function<void(result<string>)>>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, _, _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto timeout, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, timeout, move(result_handler));
        }));

    auto test_bytes_reader = make_time_limited_bytes_reader(
        *mock_evloop, mock_evloop->get_etime() + 120ns, mock_reader);

    ASSERT_EQ(saved_result_handlers.size(), 0U);

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    test_bytes_reader->read_some_bytes(4U, 100ns, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 4U);
    ASSERT_EQ(get<ch::nanoseconds>(saved_result_handlers.front()), 100ns);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("0123"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("0123"s));
    saved_result_handlers.clear();
    results.clear();

    current_time += 80ns;

    test_bytes_reader->read_some_bytes(4U, 100ns, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 4U);
    ASSERT_EQ(get<ch::nanoseconds>(saved_result_handlers.front()), 40ns);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("4567"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("4567"s));
    saved_result_handlers.clear();
    results.clear();

    current_time += 80ns;

    test_bytes_reader->read_some_bytes(4U, 100ns, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 4U);
    ASSERT_EQ(get<ch::nanoseconds>(saved_result_handlers.front()), 0ns);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_timeout_raw_error_result().as_result<string>());
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_timeout_raw_error_result().as_result<string>());
    saved_result_handlers.clear();
    results.clear();
}
