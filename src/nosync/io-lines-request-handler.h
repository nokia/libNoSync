// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__IO_LINES_REQUEST_HANDLER_H
#define NOSYNC__IO_LINES_REQUEST_HANDLER_H

#include <nosync/bytes-io.h>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<request_handler<std::string, std::string>> make_io_lines_request_handler(
    event_loop &evloop, const std::shared_ptr<bytes_io> &lines_io);

}

#endif /* NOSYNC__IO_LINES_REQUEST_HANDLER_H */
