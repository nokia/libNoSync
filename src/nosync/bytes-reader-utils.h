// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_READER_UTILS_H
#define NOSYNC__BYTES_READER_UTILS_H

#include <chrono>
#include <experimental/optional>
#include <functional>
#include <nosync/bytes-reader.h>
#include <nosync/event-loop.h>
#include <nosync/result-handler.h>
#include <memory>
#include <string>
#include <tuple>
#include <vector>


namespace nosync
{

void read_some_bytes(
    const std::shared_ptr<bytes_reader> &reader, std::size_t max_size,
    result_handler<std::string> &&res_handler);

void read_all_pending_bytes(
    std::shared_ptr<bytes_reader> &&reader,
    std::function<void(std::string)> &&data_consumer,
    std::function<void()> &&after_func);

void read_bytes_fully(
    std::shared_ptr<bytes_reader> &&reader, std::size_t size,
    result_handler<std::string> &&res_handler);

template<typename ...T, typename F>
void read_be_numbers_fully(
    std::shared_ptr<bytes_reader> &&reader,
    F &&res_handler);

template<typename T>
void read_be_numbers_fully(
    std::shared_ptr<bytes_reader> reader, std::size_t count,
    result_handler<std::vector<T>> &&res_handler);

template<typename T>
void read_be_number_fully(std::shared_ptr<bytes_reader> reader, result_handler<T> &&res_handler);

template<typename Res>
void process_read_bytes_with_timeout(
    event_loop &evloop, std::shared_ptr<bytes_reader> &&reader,
    std::chrono::nanoseconds timeout,
    std::function<std::experimental::optional<Res>(char)> &&bytes_processor,
    result_handler<Res> &&res_handler);

template<typename F>
void invoke_later_via_bytes_reader(bytes_reader &reader, F &&task);

template<typename Res>
void invoke_result_handler_later_via_bytes_reader(
    bytes_reader &reader, result_handler<Res> &&res_handler, result<Res> &&res);

}

#include <nosync/bytes-reader-utils-impl.h>

#endif /* NOSYNC__BYTES_READER_UTILS_H */
