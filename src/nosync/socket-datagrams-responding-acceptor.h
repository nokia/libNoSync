// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SOCKET_DATAGRAMS_RESPONDING_ACCEPTOR_H
#define NOSYNC__SOCKET_DATAGRAMS_RESPONDING_ACCEPTOR_H

#include <cstddef>
#include <functional>
#include <memory>
#include <nosync/bytes-io.h>
#include <nosync/fd-watching-event-loop.h>
#include <nosync/interface-type.h>
#include <nosync/owned-fd.h>
#include <string>


namespace nosync
{

std::shared_ptr<interface_type> make_socket_datagrams_responding_acceptor(
    fd_watching_event_loop &evloop, owned_fd &&sock_fd, std::size_t max_request_datagram_size,
    std::function<void(std::shared_ptr<bytes_io> &&)> &&datagrams_processor);

}

#endif /* NOSYNC__SOCKET_DATAGRAMS_RESPONDING_ACCEPTOR_H */
