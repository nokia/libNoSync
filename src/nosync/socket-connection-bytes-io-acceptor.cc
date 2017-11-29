// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/socket-connections-bytes-io-acceptor.h>
#include <utility>

using std::function;
using std::move;
using std::shared_ptr;
using std::size_t;


namespace nosync
{

shared_ptr<interface_type> make_socket_connection_bytes_io_acceptor(
    fd_watching_event_loop &evloop, owned_fd &&listen_sock_fd,
    function<void(shared_ptr<bytes_io>)> &&io_handler, size_t read_buffer_size)
{
    return make_socket_connections_bytes_io_acceptor(
        evloop, move(listen_sock_fd), move(io_handler), read_buffer_size);
}

}
