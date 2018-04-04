// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/buffered-bytes-reader.h>
#include <nosync/fd-bytes-reader.h>
#include <nosync/fd-bytes-writer.h>
#include <nosync/reader-writer-bytes-io.h>
#include <nosync/socket-connections-bytes-io-acceptor.h>
#include <nosync/socket-connections-fd-acceptor.h>
#include <nosync/type-utils.h>
#include <utility>

using std::function;
using std::move;
using std::shared_ptr;
using std::size_t;


namespace nosync
{

namespace
{

shared_ptr<bytes_io> make_fd_bytes_io(fd_watching_event_loop &evloop, shared_fd &&fd, size_t read_buffer_size)
{
    return make_reader_writer_bytes_io(
        make_buffered_bytes_reader(read_buffer_size, make_fd_bytes_reader(evloop, make_copy(fd))),
        make_fd_bytes_writer(evloop, make_copy(fd)));
}

}


shared_ptr<interface_type> make_socket_connections_bytes_io_acceptor(
    fd_watching_event_loop &evloop, owned_fd &&listen_sock_fd,
    function<void(shared_ptr<bytes_io>)> &&io_handler, size_t read_buffer_size)
{
    return make_socket_connections_fd_acceptor(
        evloop, move(listen_sock_fd),
        [&evloop, io_handler = move(io_handler), read_buffer_size](auto new_fd) {
            io_handler(make_fd_bytes_io(evloop, move(new_fd), read_buffer_size));
        });
}

}
