// This file is part of libnosync library. See LICENSE file for license details.
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nosync/bytes-io-mock.h>
#include <nosync/event-loop-utils.h>
#include <nosync/event-loop.h>
#include <nosync/manual-event-loop.h>
#include <nosync/noconcurrent-bytes-io.h>
#include <nosync/type-utils.h>
#include <stdexcept>
#include <system_error>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_io_mock;
using nosync::result;
using nosync::event_loop;
using nosync::make_copy;
using nosync::make_ok_result;
using nosync::manual_event_loop;
using std::make_shared;
using std::move;
using std::string;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Ge;
using testing::Invoke;


TEST(NosyncNoconcurrentBytesIo, CheckThrows)
{
    constexpr auto bio_tick = 10ns;
    constexpr auto bio_operation_time = bio_tick * 10;
    constexpr auto read_timeout = 1s;
    const auto read_chunk = "abc"s;
    const auto write_chunk = "def"s;

    auto noop_read_result_handler = [](result<std::string>) {
    };
    auto noop_write_result_handler = [](result<void>) {
    };

    auto evloop = manual_event_loop::create();

    auto test_bio = make_shared<bytes_io_mock>();

    EXPECT_CALL(*test_bio, read_some_bytes_impl(Ge(read_chunk.size()), Eq(read_timeout), _)).WillRepeatedly(
        Invoke(
            [&evloop, read_chunk, bio_operation_time](auto, auto, auto result_handler) {
                invoke_with_etime_delay(
                    *evloop, bio_operation_time,
                    [result_handler = move(result_handler), read_chunk]() {
                        result_handler(make_ok_result(read_chunk));
                    });
            }));
    EXPECT_CALL(*test_bio, write_bytes_impl(Eq(write_chunk), _)).WillRepeatedly(
        Invoke(
            [&evloop, bio_operation_time](auto, auto result_handler) {
                invoke_with_etime_delay(
                    *evloop, bio_operation_time,
                    [result_handler = move(result_handler)]() {
                        result_handler(make_ok_result());
                    });
            }));

    auto nc_bio = make_noconcurrent_bytes_io(test_bio);

    vector<result<string>> saved_read_results;
    nc_bio->read_some_bytes(
        read_chunk.size(), read_timeout,
        [&saved_read_results](auto result) {
            saved_read_results.push_back(result);
        });

    evloop->process_time_passage(bio_operation_time - bio_tick);

    ASSERT_THROW(
        nc_bio->read_some_bytes(read_chunk.size(), read_timeout, noop_read_result_handler),
        std::runtime_error);
    ASSERT_THROW(
        nc_bio->write_bytes(make_copy(write_chunk), noop_write_result_handler),
        std::runtime_error);

    evloop->process_time_passage(bio_tick);

    ASSERT_EQ(saved_read_results.size(), 1U);
    ASSERT_EQ(saved_read_results.front(), make_ok_result(read_chunk));

    vector<result<void>> saved_result;
    nc_bio->write_bytes(
        make_copy(write_chunk),
        [&saved_result](auto result) {
            saved_result.push_back(result);
        });

    evloop->process_time_passage(bio_operation_time - bio_tick);

    ASSERT_THROW(
        nc_bio->read_some_bytes(read_chunk.size(), read_timeout, noop_read_result_handler),
        std::runtime_error);
    ASSERT_THROW(
        nc_bio->write_bytes(make_copy(write_chunk), noop_write_result_handler),
        std::runtime_error);

    evloop->process_time_passage(bio_tick);

    ASSERT_EQ(saved_result.size(), 1U);
    ASSERT_EQ(saved_result.front(), make_ok_result());
}
