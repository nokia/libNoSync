// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SOCKET_CONNECTIONS_FD_ACCEPTOR_H
#define NOSYNC__SOCKET_CONNECTIONS_FD_ACCEPTOR_H

#include <functional>
#include <nosync/result.h>
#include <nosync/fd-watcher.h>
#include <nosync/interface-type.h>
#include <nosync/owned-fd.h>
#include <nosync/result-handler.h>
#include <nosync/shared-fd.h>
#include <memory>


namespace nosync
{

/*!
Take ownership of listening socket and handle incoming connections.

The function takes ownership of passive non-blocking socket (configured
for accepting connections) and accepts all incoming connections, for
each one calling provided handler and passing connection file descriptor
(also configured as non-blocking) to it.

New connections are being accepted as long as the returned object is
alive.
*/
std::shared_ptr<interface_type> make_full_socket_connections_fd_acceptor(
    fd_watcher &watcher, owned_fd &&listen_sock_fd,
    result_handler<shared_fd> &&conn_handler);


/*!
Simplified variant of make_full_socket_connections_fd_acceptor.

This function should be used if the user is not interested in accept()
failures. In this case the handler function gets plain shared_fd with
the accepted connection as the parameter.
*/
std::shared_ptr<interface_type> make_socket_connections_fd_acceptor(
    fd_watcher &watcher, owned_fd &&listen_sock_fd,
    std::function<void(shared_fd)> &&conn_handler);

}

#endif /* NOSYNC__SOCKET_CONNECTIONS_FD_ACCEPTOR_H */
