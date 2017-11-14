// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_IO_MOCK_H
#define NOSYNC__BYTES_IO_MOCK_H

#include <gmock/gmock.h>
#include <nosync/bytes-io.h>


namespace nosync
{

class bytes_io_no_rvref_wrapper : public bytes_io
{
public:
    bytes_io_no_rvref_wrapper() = default;

    void read_some_bytes(
        std::size_t max_size, std::chrono::nanoseconds timeout,
        result_handler<std::string> &&res_handler) override final;
    void write_bytes(std::string &&data, result_handler<void> &&res_handler) override final;

    virtual void read_some_bytes_impl(
        std::size_t max_size, std::chrono::nanoseconds timeout,
        result_handler<std::string> res_handler) = 0;
    virtual void write_bytes_impl(std::string data, result_handler<void> res_handler) = 0;
};


class bytes_io_mock : public bytes_io_no_rvref_wrapper
{
public:
    bytes_io_mock() = default;

    MOCK_METHOD3(read_some_bytes_impl, void(std::size_t, std::chrono::nanoseconds, std::function<void(result<std::string>)>));

    MOCK_METHOD2(write_bytes_impl, void(std::string, std::function<void(result<void>)>));
};

}

#include <nosync/bytes-io-mock-impl.h>

#endif /* NOSYNC__BYTES_IO_MOCK_H */
