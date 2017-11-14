// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-reader-mock.h>
#include <nosync/event-loop-mock.h>
#include <nosync/event-loop.h>
#include <nosync/ppoll-based-event-loop.h>
#include <nosync/result.h>
#include <nosync/sequential-bytes-reader.h>
#include <string>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_reader_mock;
using nosync::event_loop_mock;
using nosync::make_ok_result;
using nosync::make_sequential_bytes_reader;
using nosync::result;
using std::function;
using std::make_shared;
using std::move;
using std::string;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Invoke;


namespace
{

constexpr auto very_long_timeout = 1h;

}


TEST(NosyncSequentialBytesReader, SimpleRead)
{
    auto mock_evloop = make_shared<event_loop_mock>();

    constexpr auto test_max_size = 10U;
    constexpr auto test_timeout = 3ns;
    const auto test_data = "abc"s;

    vector<function<void(result<string>)>> saved_read_result_handlers;
    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(test_max_size), Eq(test_timeout), _)).WillRepeatedly(Invoke(
        [&saved_read_result_handlers](auto, auto, auto result_handler) {
            saved_read_result_handlers.push_back(move(result_handler));
        }));

    auto reader = make_sequential_bytes_reader(*mock_evloop, mock_reader);

    reader->read_some_bytes(
        test_max_size, test_timeout,
        [test_data](auto result) {
            ASSERT_EQ(result, make_ok_result(test_data));
        });

    ASSERT_EQ(saved_read_result_handlers.size(), 1U);
    saved_read_result_handlers.front()(make_ok_result(test_data));
}
