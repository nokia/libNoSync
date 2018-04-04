// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_WRITER_MOCK_H
#define NOSYNC__BYTES_WRITER_MOCK_H

#include <gmock/gmock.h>
#include <nosync/bytes-writer.h>


namespace nosync
{

class bytes_writer_no_rvref_wrapper : public bytes_writer
{
public:
    bytes_writer_no_rvref_wrapper() = default;

    void write_bytes(std::string &&data, result_handler<void> &&res_handler) override final;

    virtual void write_bytes_impl(std::string data, result_handler<void> res_handler) = 0;
};


class bytes_writer_mock : public bytes_writer_no_rvref_wrapper
{
public:
    bytes_writer_mock() = default;

    MOCK_METHOD2(write_bytes_impl, void(std::string, std::function<void(result<void>)>));
};

}

#include <nosync/bytes-writer-mock-impl.h>

#endif /* NOSYNC__BYTES_WRITER_MOCK_H */
