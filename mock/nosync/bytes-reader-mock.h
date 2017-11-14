// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_READER_MOCK_H
#define NOSYNC__BYTES_READER_MOCK_H

#include <gmock/gmock.h>
#include <nosync/bytes-reader.h>


namespace nosync
{

class bytes_reader_no_rvref_wrapper : public bytes_reader
{
public:
    bytes_reader_no_rvref_wrapper() = default;

    void read_some_bytes(
        std::size_t max_size, std::chrono::nanoseconds timeout,
        result_handler<std::string> &&res_handler) override final;

    virtual void read_some_bytes_impl(
        std::size_t max_size, std::chrono::nanoseconds timeout,
        result_handler<std::string> res_handler) = 0;
};


class bytes_reader_mock : public bytes_reader_no_rvref_wrapper
{
public:
    bytes_reader_mock() = default;

    MOCK_METHOD3(read_some_bytes_impl, void(std::size_t, std::chrono::nanoseconds, std::function<void(result<std::string>)>));
};

}

#include <nosync/bytes-reader-mock-impl.h>

#endif /* NOSYNC__BYTES_READER_MOCK_H */
