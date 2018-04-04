// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FUNC_BYTES_WRITER_H
#define NOSYNC__FUNC_BYTES_WRITER_H

#include <functional>
#include <nosync/bytes-writer.h>
#include <nosync/result-handler.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_writer> make_func_bytes_writer(
    std::function<void(std::string &&, result_handler<void> &&)> &&writer_func);

}

#endif /* NOSYNC__FUNC_BYTES_WRITER_H */
