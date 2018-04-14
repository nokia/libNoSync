// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_BYTES_IO_H
#define NOSYNC__FD_BYTES_IO_H

#include <cstddef>
#include <nosync/bytes-io.h>
#include <nosync/fd-watching-event-loop.h>
#include <nosync/shared-fd.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_io> make_fd_bytes_io(
    fd_watching_event_loop &evloop, shared_fd &&fd, std::size_t read_buffer_size = 4096);

}

#endif /* NOSYNC__FD_BYTES_IO_H */
