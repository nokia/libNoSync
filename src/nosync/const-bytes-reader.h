// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__CONST_BYTES_READER_H
#define NOSYNC__CONST_BYTES_READER_H

#include <nosync/bytes-reader.h>
#include <nosync/event-loop.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<bytes_reader> make_const_bytes_reader(event_loop &evloop, std::string &&input_bytes);

}

#endif /* NOSYNC__CONST_BYTES_READER_H */
