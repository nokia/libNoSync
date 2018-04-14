// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SOCKET_CONNECTIONS_FD_REQUESTER_H
#define NOSYNC__SOCKET_CONNECTIONS_FD_REQUESTER_H

#include <cstddef>
#include <memory>
#include <nosync/fd-watching-event-loop.h>
#include <nosync/request-handler.h>
#include <nosync/shared-fd.h>
#include <nosync/socket-address.h>


namespace nosync
{

void request_socket_connection_fd(
    fd_watching_event_loop &evloop, const std::shared_ptr<socket_address> &addr,
    std::chrono::nanoseconds timeout, result_handler<shared_fd> &&res_handler);

std::shared_ptr<request_handler<std::shared_ptr<socket_address>, shared_fd>> make_socket_connections_fd_requester(
    fd_watching_event_loop &evloop);

}

#endif /* NOSYNC__SOCKET_CONNECTIONS_FD_REQUESTER_H */
