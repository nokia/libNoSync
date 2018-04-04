// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_BYTES_READER_H
#define NOSYNC__FD_BYTES_READER_H

#include <nosync/bytes-reader.h>
#include <nosync/fd-watching-event-loop.h>
#include <nosync/shared-fd.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_reader> make_fd_bytes_reader(fd_watching_event_loop &evloop, shared_fd &&fd);

}

#endif /* NOSYNC__FD_BYTES_READER_H */
