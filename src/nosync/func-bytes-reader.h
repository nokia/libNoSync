// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FUNC_BYTES_READER_H
#define NOSYNC__FUNC_BYTES_READER_H

#include <cstddef>
#include <functional>
#include <nosync/bytes-reader.h>
#include <nosync/result-handler.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_reader> make_func_bytes_reader(
    std::function<void(std::size_t, std::chrono::nanoseconds, result_handler<std::string>)> &&reader_func);

}

#endif /* NOSYNC__FUNC_BYTES_READER_H */
