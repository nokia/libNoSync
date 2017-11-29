// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SOCKET_CONNECTION_BYTES_IO_ACCEPTOR_H
#define NOSYNC__SOCKET_CONNECTION_BYTES_IO_ACCEPTOR_H

#include <functional>
#include <nosync/bytes-io.h>
#include <nosync/fd-watching-event-loop.h>
#include <nosync/interface-type.h>
#include <nosync/owned-fd.h>
#include <memory>


namespace nosync
{

std::shared_ptr<interface_type> make_socket_connection_bytes_io_acceptor(
    fd_watching_event_loop &evloop, owned_fd &&listen_sock_fd,
    std::function<void(std::shared_ptr<bytes_io>)> &&io_handler,
    std::size_t read_buffer_size = 8192);

}

#endif /* NOSYNC__SOCKET_CONNECTION_BYTES_IO_ACCEPTOR_H */
