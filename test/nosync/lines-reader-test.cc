// This file is part of libnosync library. See LICENSE file for license details.
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-reader-mock.h>
#include <nosync/const-bytes-reader.h>
#include <nosync/event-loop-utils.h>
#include <nosync/event-loop.h>
#include <nosync/lines-reader.h>
#include <nosync/manual-event-loop.h>
#include <nosync/result-utils.h>
#include <nosync/result.h>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_reader_mock;
using nosync::manual_event_loop;
using nosync::make_const_bytes_reader;
using nosync::make_error_result;
using nosync::make_lines_reader;
using nosync::make_ok_result;
using nosync::make_timeout_raw_error_result;
using nosync::request_handler;
using nosync::result;
using std::errc;
using std::function;
using std::make_error_code;
using std::make_shared;
using std::move;
using std::nullptr_t;
using std::shared_ptr;
using std::string;
using std::vector;
using testing::_;
using testing::Ge;
using testing::Invoke;


namespace
{

constexpr auto test_req_timeout = 10ns;


template<typename Req, typename Res>
void repeat_request_handler_calls(
    shared_ptr<request_handler<Req, Res>> req_handler,
    unsigned repeat_count,
    function<void(request_handler<Req, Res> &, function<void(result<string>)>)> req_caller,
    function<void(result<string>)> res_handler)
{
    if (repeat_count != 0) {
        req_caller(
            *req_handler,
            [req_handler, repeat_count, req_caller, res_handler = move(res_handler)](auto res) mutable {
                res_handler(move(res));
                repeat_request_handler_calls(req_handler, repeat_count - 1, move(req_caller), move(res_handler));
            });
    }
}


template<typename T>
function<void(T)> make_vector_pusher(vector<T> &out_vector)
{
    return [&out_vector](auto value) {
        out_vector.push_back(move(value));
    };
}

}


TEST(NosyncLinesReader, ValidLines)
{
    auto evloop = manual_event_loop::create();

    auto lines_reader = make_lines_reader(
        *evloop, make_const_bytes_reader(*evloop, "\n\n123\n456789\n\nabcdefg\n"s), 7);

    vector<result<string>> saved_results;
    repeat_request_handler_calls<nullptr_t, string>(
        lines_reader, 6,
        [](auto &lines_reader, auto res_handler) {
            lines_reader.handle_request(nullptr, test_req_timeout, move(res_handler));
        },
        make_vector_pusher(saved_results));

    evloop->process_time_passage(1ns);

    ASSERT_EQ(saved_results.size(), 6U);
    ASSERT_EQ(saved_results[0], make_ok_result(""s));
    ASSERT_EQ(saved_results[1], make_ok_result(""s));
    ASSERT_EQ(saved_results[2], make_ok_result("123"s));
    ASSERT_EQ(saved_results[3], make_ok_result("456789"s));
    ASSERT_EQ(saved_results[4], make_ok_result(""s));
    ASSERT_EQ(saved_results[5], make_ok_result("abcdefg"s));
}


TEST(NosyncLinesReader, ValidLinesWithBigMax)
{
    auto evloop = manual_event_loop::create();

    auto lines_reader = make_lines_reader(
        *evloop, make_const_bytes_reader(*evloop, "\n\n123\n"s), 1024 * 1024);

    vector<result<string>> saved_results;
    repeat_request_handler_calls<nullptr_t, string>(
        lines_reader, 3,
        [](auto &lines_reader, auto res_handler) {
            lines_reader.handle_request(nullptr, test_req_timeout, move(res_handler));
        },
        make_vector_pusher(saved_results));

    evloop->process_time_passage(1ns);

    ASSERT_EQ(saved_results.size(), 3U);
    ASSERT_EQ(saved_results[0], make_ok_result(""s));
    ASSERT_EQ(saved_results[1], make_ok_result(""s));
    ASSERT_EQ(saved_results[2], make_ok_result("123"s));
}


TEST(NosyncLinesReader, Eof)
{
    auto evloop = manual_event_loop::create();

    auto lines_reader = make_lines_reader(
        *evloop, make_const_bytes_reader(*evloop, "123\n456"s), 3);

    vector<result<string>> saved_results;
    repeat_request_handler_calls<nullptr_t, string>(
        lines_reader, 3,
        [](auto &lines_reader, auto res_handler) {
            lines_reader.handle_request(nullptr, test_req_timeout, move(res_handler));
        },
        make_vector_pusher(saved_results));

    evloop->process_time_passage(1ns);

    ASSERT_EQ(saved_results.size(), 3U);
    ASSERT_EQ(saved_results[0], make_ok_result("123"s));
    ASSERT_EQ(saved_results[1], make_error_result<string>(make_error_code(errc::no_message_available)));
    ASSERT_EQ(saved_results[2], make_error_result<string>(make_error_code(errc::no_message_available)));
}


TEST(NosyncLinesReader, TooLongLine)
{
    auto evloop = manual_event_loop::create();

    auto lines_reader = make_lines_reader(
        *evloop, make_const_bytes_reader(*evloop, "123\n456\nabcdefghi\n"s), 3);

    vector<result<string>> saved_results;
    repeat_request_handler_calls<nullptr_t, string>(
        lines_reader, 3,
        [](auto &lines_reader, auto res_handler) {
            lines_reader.handle_request(nullptr, test_req_timeout, move(res_handler));
        },
        make_vector_pusher(saved_results));

    evloop->process_time_passage(1ns);

    ASSERT_EQ(saved_results.size(), 3U);
    ASSERT_EQ(saved_results[0], make_ok_result("123"s));
    ASSERT_EQ(saved_results[1], make_ok_result("456"s));
    ASSERT_EQ(saved_results[2], make_error_result<string>(make_error_code(errc::file_too_large)));
}


TEST(NosyncLinesReader, ReadError)
{
    auto evloop = manual_event_loop::create();

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, _, _)).WillRepeatedly(Invoke(
        [&evloop](auto, auto, auto result_handler) {
            evloop->invoke_at(
                evloop->get_etime(),
                [result_handler = move(result_handler)]() {
                    result_handler(make_error_result<string>(make_error_code(errc::io_error)));
                });
        }));
    auto lines_reader = make_lines_reader(*evloop, mock_reader, 10);

    vector<result<string>> saved_results;
    repeat_request_handler_calls<nullptr_t, string>(
        lines_reader, 1,
        [](auto &lines_reader, auto res_handler) {
            lines_reader.handle_request(nullptr, test_req_timeout, move(res_handler));
        },
        make_vector_pusher(saved_results));

    evloop->process_time_passage(1ns);

    ASSERT_EQ(saved_results.size(), 1U);
    ASSERT_EQ(saved_results[0], make_error_result<string>(make_error_code(errc::io_error)));
}


TEST(NosyncLinesReader, Timeout)
{
    auto evloop = manual_event_loop::create();

    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Ge(1), _, _)).WillRepeatedly(Invoke(
        [&evloop](auto, auto timeout, auto result_handler) {
            if (timeout >= 10ns) {
                evloop->invoke_at(
                    evloop->get_etime() + 10ns,
                    [result_handler = move(result_handler)]() {
                        result_handler(make_ok_result("\n"s));
                    });
            } else {
                evloop->invoke_at(
                    evloop->get_etime() + timeout,
                    [result_handler = move(result_handler)]() {
                        result_handler(make_timeout_raw_error_result().as_result<string>());
                    });
            }
        }));
    auto lines_reader = make_lines_reader(*evloop, mock_reader, 10);

    vector<result<string>> saved_results;
    repeat_request_handler_calls<nullptr_t, string>(
        lines_reader, 2,
        [&](auto &lines_reader, auto res_handler) {
            lines_reader.handle_request(nullptr, saved_results.empty() ? 20ns : 5ns, move(res_handler));
        },
        make_vector_pusher(saved_results));

    evloop->process_time_passage(0ns);
    evloop->process_time_passage(10ns);
    evloop->process_time_passage(5ns);

    ASSERT_EQ(saved_results.size(), 2U);
    ASSERT_EQ(saved_results[0], make_ok_result(""s));
    ASSERT_EQ(saved_results[1], make_timeout_raw_error_result().as_result<string>());
}
