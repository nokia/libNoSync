// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FD_BYTES_WRITER_H
#define NOSYNC__FD_BYTES_WRITER_H

#include <nosync/bytes-writer.h>
#include <nosync/fd-watcher.h>
#include <nosync/shared-fd.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_writer> make_fd_bytes_writer(fd_watcher &watcher, shared_fd &&fd);

}

#endif /* NOSYNC__FD_BYTES_WRITER_H */
