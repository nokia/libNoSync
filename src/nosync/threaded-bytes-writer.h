// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__THREADED_BYTES_WRITER_H
#define NOSYNC__THREADED_BYTES_WRITER_H

#include <functional>
#include <nosync/bytes-writer.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<bytes_writer> make_threaded_bytes_writer(
    std::function<void(std::function<void()>)> &&evloop_mt_executor,
    std::function<void(std::function<void()>)> &&thread_executor,
    std::function<result<void>(std::string)> &&sync_writer);

}

#endif /* NOSYNC__THREADED_BYTES_WRITER_H */
