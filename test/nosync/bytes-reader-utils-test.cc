// This file is part of libnosync library. See LICENSE file for license details.
#include <experimental/optional>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-reader-mock.h>
#include <nosync/bytes-reader-utils.h>
#include <nosync/event-loop-utils.h>
#include <nosync/manual-event-loop.h>
#include <nosync/result.h>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_reader_mock;
using nosync::read_all_pending_bytes;
using nosync::result;
using nosync::event_loop;
using nosync::make_error_result;
using nosync::make_ok_result;
using nosync::manual_event_loop;
using nosync::process_read_bytes_with_timeout;
using std::errc;
using std::error_code;
using std::experimental::optional;
using std::function;
using std::make_error_code;
using std::make_shared;
using std::move;
using std::ref;
using std::size_t;
using std::string;
using std::vector;
using testing::_;
using testing::Gt;
using testing::Invoke;
using testing::SaveArg;


namespace
{

function<optional<string>(char)> make_byte_group_searching_bytes_processor(char search_byte)
{
    return [search_byte, group_size = 0U](char byte) mutable {
        optional<string> result;
        if (byte == search_byte) {
            ++group_size;
        } else if (group_size != 0U) {
            result = string(group_size, search_byte);
            group_size = 0U;
        }

        return result;
    };
}


function<void(size_t, ch::nanoseconds, function<void(result<string>)>)>
make_regular_bytes_reader(event_loop &evloop, string input_bytes, ch::nanoseconds byte_read_time)
{
    return [&evloop, input_bytes, byte_read_time](auto max_read_size, auto timeout, auto result_handler) mutable {
        auto result = make_error_result<string>(error_code());
        if (timeout < byte_read_time || input_bytes.empty()) {
            result = make_error_result<string>(make_error_code(errc::timed_out));
        } else if (max_read_size < 1) {
            result = make_ok_result(""s);
        } else {
            result = make_ok_result(input_bytes.substr(0, 1));
            input_bytes.erase(0, 1);
        }

        invoke_with_etime_delay(
            evloop, byte_read_time,
            [result_handler = move(result_handler), result = move(result)]() {
                result_handler(result);
            });

    };
}

}


TEST(NosyncBytesReadUtils, NosyncReadAllPendingDataAndThenWithNoPending)
{
    auto mock_reader = make_shared<bytes_reader_mock>();

    function<void(result<string>)> saved_read_result_handler;
    vector<string> data_chunks;
    bool got_continue_call = false;

    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Gt(0), 0ns, _)).WillOnce(SaveArg<2>(&saved_read_result_handler));
    read_all_pending_bytes(
        mock_reader,
        [&](auto data) {
            data_chunks.push_back(data);
        },
        [&]() {
            got_continue_call = true;
        });

    ASSERT_FALSE(got_continue_call);

    ASSERT_TRUE(saved_read_result_handler);
    auto read_result_handler = move(saved_read_result_handler);
    saved_read_result_handler = nullptr;
    read_result_handler(make_error_result<string>(make_error_code(errc::timed_out)));

    ASSERT_FALSE(saved_read_result_handler);
    ASSERT_TRUE(got_continue_call);
    ASSERT_EQ(data_chunks, vector<string>());
}


TEST(NosyncBytesReadUtils, NosyncReadAllPendingDataAndThenWithPending)
{
    auto mock_reader = make_shared<bytes_reader_mock>();

    function<void(result<string>)> saved_read_result_handler;
    vector<string> data_chunks;
    bool got_continue_call = false;

    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Gt(0), 0ns, _)).WillRepeatedly(SaveArg<2>(&saved_read_result_handler));
    read_all_pending_bytes(
        mock_reader,
        [&](auto data) {
            data_chunks.push_back(data);
        },
        [&]() {
            got_continue_call = true;
        });

    ASSERT_FALSE(got_continue_call);

    ASSERT_TRUE(saved_read_result_handler);
    auto read_result_handler_1 = move(saved_read_result_handler);
    saved_read_result_handler = nullptr;
    read_result_handler_1(make_ok_result("abcd"s));
    read_result_handler_1 = nullptr;

    ASSERT_TRUE(saved_read_result_handler);
    auto read_result_handler_2 = move(saved_read_result_handler);
    saved_read_result_handler = nullptr;
    read_result_handler_2(make_ok_result("efgh"s));
    read_result_handler_2 = nullptr;

    ASSERT_TRUE(saved_read_result_handler);
    auto read_result_handler_3 = move(saved_read_result_handler);
    saved_read_result_handler = nullptr;
    read_result_handler_3(make_error_result<string>(make_error_code(errc::timed_out)));
    read_result_handler_3 = nullptr;

    ASSERT_FALSE(saved_read_result_handler);
    ASSERT_TRUE(got_continue_call);
    ASSERT_EQ(data_chunks, vector<string>({"abcd"s, "efgh"s}));
}


TEST(NosyncBytesReadUtils, ProcessNosyncReadBytesWithTimeoutGetResult)
{
    auto evloop = manual_event_loop::create();
    auto mock_reader = make_shared<bytes_reader_mock>();

    constexpr auto byte_read_time = 1s;
    const auto bytes_group = "aaa"s;
    const auto input_bytes = "."s + bytes_group + "."s;
    auto bytes_reader = make_regular_bytes_reader(*evloop, input_bytes, byte_read_time);

    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, _, _)).WillRepeatedly(Invoke(ref(bytes_reader)));

    auto proc_result = make_error_result<string>(error_code());;

    process_read_bytes_with_timeout<string>(
        *evloop, mock_reader, input_bytes.size() * byte_read_time,
        make_byte_group_searching_bytes_processor('a'),
        [&](auto result) {
            proc_result = move(result);
        });

    for (auto i = input_bytes.size() + 1; i != 0; --i) {
        evloop->process_time_passage(byte_read_time);
    }

    ASSERT_TRUE(proc_result.is_ok());
    ASSERT_EQ(proc_result.get_value(), bytes_group);
}


TEST(NosyncBytesReadUtils, ProcessNosyncReadBytesWithTimeoutGetTimeout)
{
    auto evloop = manual_event_loop::create();
    auto mock_reader = make_shared<bytes_reader_mock>();

    constexpr auto byte_read_time = 1s;
    const auto input_bytes = "abcdefghi"s;
    auto bytes_reader = make_regular_bytes_reader(*evloop, input_bytes, byte_read_time);

    EXPECT_CALL(*mock_reader, read_some_bytes_impl(_, _, _)).WillRepeatedly(Invoke(ref(bytes_reader)));

    auto proc_result = make_error_result<string>(error_code());;

    process_read_bytes_with_timeout<string>(
        *evloop, mock_reader, input_bytes.size() * byte_read_time,
        make_byte_group_searching_bytes_processor('X'),
        [&](auto result) {
            proc_result = move(result);
        });

    for (auto i = input_bytes.size() + 1; i != 0; --i) {
        evloop->process_time_passage(byte_read_time);
    }

    ASSERT_FALSE(proc_result.is_ok());
    ASSERT_EQ(proc_result.get_error(), make_error_code(errc::timed_out));
}
