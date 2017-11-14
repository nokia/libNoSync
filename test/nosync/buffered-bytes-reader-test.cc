// This file is part of libnosync library. See LICENSE file for license details.
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/buffered-bytes-reader.h>
#include <nosync/bytes-reader-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/result.h>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_reader_mock;
using nosync::eclock;
using nosync::event_loop_mock;
using nosync::make_buffered_bytes_reader;
using nosync::make_error_result;
using nosync::make_ok_result;
using nosync::result;
using std::errc;
using std::function;
using std::get;
using std::make_error_code;
using std::make_shared;
using std::move;
using std::size_t;
using std::string;
using std::tuple;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Invoke;
using testing::Return;


TEST(NosyncBufferedBytesReader, SmallReadsOk)
{
    constexpr auto read_timeout = 3ns;
    constexpr auto buffer_size = 10U;

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(ch::time_point<eclock>(123s)));

    vector<tuple<size_t, function<void(result<string>)>>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(0U), Eq(ch::nanoseconds::max()), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(buffer_size), Eq(read_timeout), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));

    auto test_reader = make_buffered_bytes_reader(buffer_size, mock_reader);

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    ASSERT_EQ(saved_result_handlers.size(), 0U);

    test_reader->read_some_bytes(5, read_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 10U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("0123456789"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("01234"s));
    saved_result_handlers.clear();
    results.clear();

    test_reader->read_some_bytes(2, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 0U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result(""s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("56"s));
    saved_result_handlers.clear();
    results.clear();

    test_reader->read_some_bytes(5, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 0U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result(""s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("789"s));
    saved_result_handlers.clear();
    results.clear();

    test_reader->read_some_bytes(5, read_timeout, result_pusher);
    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 10U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("ABCDE"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("ABCDE"s));
    saved_result_handlers.clear();
    results.clear();
}


TEST(NosyncBufferedBytesReader, BigReadOk)
{
    constexpr auto read_timeout = 3ns;
    constexpr auto buffer_size = 10U;

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(ch::time_point<eclock>(123s)));

    vector<tuple<size_t, function<void(result<string>)>>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(0U), Eq(0ns), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, Eq(read_timeout), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));

    auto test_reader = make_buffered_bytes_reader(buffer_size, mock_reader);

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    ASSERT_EQ(saved_result_handlers.size(), 0U);

    test_reader->read_some_bytes(10, read_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 10U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("0123456789"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("0123456789"s));
    saved_result_handlers.clear();
    results.clear();

    test_reader->read_some_bytes(15, read_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 15U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("ABCDEFGHIJKLMNO"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("ABCDEFGHIJKLMNO"s));
    saved_result_handlers.clear();
    results.clear();
}


TEST(NosyncBufferedBytesReader, ReadFailure)
{
    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(ch::time_point<eclock>(123s)));

    vector<function<void(result<string>)>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, _, _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto, auto, auto result_handler) {
            saved_result_handlers.push_back(move(result_handler));
        }));

    auto test_reader = make_buffered_bytes_reader(10U, mock_reader);

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    ASSERT_EQ(saved_result_handlers.size(), 0U);

    test_reader->read_some_bytes(5, 3ns, result_pusher);

    const auto test_error_result = make_error_result<string>(make_error_code(errc::bad_message));

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    saved_result_handlers.front()(test_error_result);
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), test_error_result);
    saved_result_handlers.clear();
    results.clear();
}


TEST(NosyncBufferedBytesReader, ZeroBuffer)
{
    constexpr auto read_timeout = 3ns;

    auto mock_evloop = make_shared<event_loop_mock>();
    EXPECT_CALL(*mock_evloop, get_etime()).WillRepeatedly(Return(ch::time_point<eclock>(123s)));

    vector<tuple<size_t, function<void(result<string>)>>> saved_result_handlers;

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, Eq(read_timeout), _)).WillRepeatedly(Invoke(
        [&saved_result_handlers](auto max_size, auto, auto result_handler) {
            saved_result_handlers.emplace_back(max_size, move(result_handler));
        }));

    auto test_reader = make_buffered_bytes_reader(0, mock_reader);

    vector<result<string>> results;
    auto result_pusher = [&results](auto result) {
        results.push_back(move(result));
    };

    ASSERT_EQ(saved_result_handlers.size(), 0U);

    test_reader->read_some_bytes(5, read_timeout, result_pusher);

    ASSERT_EQ(results.size(), 0U);
    ASSERT_EQ(saved_result_handlers.size(), 1U);
    ASSERT_EQ(get<size_t>(saved_result_handlers.front()), 5U);
    get<function<void(result<string>)>>(saved_result_handlers.front())(make_ok_result("01234"s));
    ASSERT_EQ(results.size(), 1U);
    ASSERT_EQ(results.front(), make_ok_result("01234"s));
    saved_result_handlers.clear();
    results.clear();
}
