// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SEQUENTIAL_BYTES_READER_H
#define NOSYNC__SEQUENTIAL_BYTES_READER_H

#include <nosync/bytes-reader.h>
#include <nosync/event-loop.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_reader> make_sequential_bytes_reader(
    event_loop &evloop, std::shared_ptr<bytes_reader> &&base_reader);

}

#endif /* NOSYNC__SEQUENTIAL_BYTES_READER_H */
