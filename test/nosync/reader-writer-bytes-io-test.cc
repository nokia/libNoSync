// This file is part of libnosync library. See LICENSE file for license details.
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-reader-mock.h>
#include <nosync/bytes-writer-mock.h>
#include <nosync/reader-writer-bytes-io.h>
#include <nosync/result-utils.h>
#include <nosync/type-utils.h>
#include <string>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::bytes_reader_mock;
using nosync::bytes_writer_mock;
using nosync::make_copy;
using nosync::make_ok_result;
using nosync::make_reader_writer_bytes_io;
using nosync::make_timeout_raw_error_result;
using nosync::result;
using std::function;
using std::make_shared;
using std::move;
using std::string;
using std::vector;
using testing::_;
using testing::Eq;
using testing::Invoke;


TEST(NosyncReaderWriterBytesIo, TestRead)
{
    const auto test_read_max_size = 100U;
    const auto test_read_timeout = 12ns;

    vector<function<void(result<string>)>> saved_read_res_handlers;
    auto mock_reader = make_shared<bytes_reader_mock>();
    EXPECT_CALL(*mock_reader, read_some_bytes_impl(Eq(test_read_max_size), Eq(test_read_timeout), _)).WillOnce(Invoke(
        [&saved_read_res_handlers](auto, auto, auto res_handler) {
            saved_read_res_handlers.emplace_back(move(res_handler));
        }));

    auto mock_writer = make_shared<bytes_writer_mock>();

    auto test_bio = make_reader_writer_bytes_io(mock_reader, mock_writer);

    vector<result<string>> saved_read_results;
    test_bio->read_some_bytes(
        test_read_max_size, test_read_timeout,
        [&saved_read_results](auto read_res) {
            saved_read_results.push_back(move(read_res));
        });

    ASSERT_EQ(saved_read_results.size(), 0U);
    ASSERT_EQ(saved_read_res_handlers.size(), 1U);

    const auto test_result = make_ok_result("123"s);

    saved_read_res_handlers.front()(test_result);
    ASSERT_EQ(saved_read_results.size(), 1U);
    ASSERT_EQ(saved_read_results.front(), test_result);
}


TEST(NosyncReaderWriterBytesIo, TestWrite)
{
    const auto test_write_data = "abc"s;

    auto mock_reader = make_shared<bytes_reader_mock>();

    vector<function<void(result<void>)>> saved_write_res_handlers;
    auto mock_writer = make_shared<bytes_writer_mock>();
    EXPECT_CALL(*mock_writer, write_bytes_impl(Eq(test_write_data), _)).WillOnce(Invoke(
        [&saved_write_res_handlers](auto, auto res_handler) {
            saved_write_res_handlers.emplace_back(move(res_handler));
        }));

    auto test_bio = make_reader_writer_bytes_io(mock_reader, mock_writer);

    vector<result<void>> saved_write_results;
    test_bio->write_bytes(
        make_copy(test_write_data),
        [&saved_write_results](auto write_res) {
            saved_write_results.push_back(move(write_res));
        });

    ASSERT_EQ(saved_write_results.size(), 0U);
    ASSERT_EQ(saved_write_res_handlers.size(), 1U);

    const auto test_result = make_timeout_raw_error_result().as_result<void>();

    saved_write_res_handlers.front()(test_result);
    ASSERT_EQ(saved_write_results.size(), 1U);
    ASSERT_EQ(saved_write_results.front(), test_result);
}
