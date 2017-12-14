// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SEQUENTIAL_CHUNKS_WRITER_H
#define NOSYNC__SEQUENTIAL_CHUNKS_WRITER_H

#include <nosync/event-loop.h>
#include <nosync/bytes-writer.h>
#include <nosync/request-handler.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<request_handler<std::string, void>> make_sequential_chunks_writer(
    event_loop &evloop, std::shared_ptr<bytes_writer> &&base_writer);

}

#endif /* NOSYNC__SEQUENTIAL_CHUNKS_WRITER_H */
