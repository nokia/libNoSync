// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__LINES_READER_H
#define NOSYNC__LINES_READER_H

#include <cstddef>
#include <nosync/event-loop.h>
#include <nosync/bytes-reader.h>
#include <nosync/request-handler.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<request_handler<std::nullptr_t, std::string>> make_lines_reader(
    event_loop &evloop, std::shared_ptr<bytes_reader> base_reader, std::size_t max_line_size = 1024 * 1024);

}

#endif /* NOSYNC__LINES_READER_H */
