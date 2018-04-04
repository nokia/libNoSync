// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/result-utils.h>
#include <nosync/socket-connections-fd-acceptor.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <utility>

using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::unique_ptr;


namespace nosync
{

namespace
{

class socket_connections_fd_acceptor : public interface_type
{
public:
    socket_connections_fd_acceptor(
        fd_watcher &watcher, owned_fd &&listen_sock_fd, result_handler<shared_fd> &&conn_handler);
    ~socket_connections_fd_acceptor() override;

private:
    owned_fd listen_sock_fd;
    unique_ptr<activity_handle> listen_sock_watch_handle;
};


socket_connections_fd_acceptor::socket_connections_fd_acceptor(
    fd_watcher &watcher, owned_fd &&listen_sock_fd, result_handler<shared_fd> &&conn_handler)
    : listen_sock_fd(move(listen_sock_fd))
{
    listen_sock_watch_handle = watcher.add_watch(
        *this->listen_sock_fd, fd_watch_mode::input,
        [fd = *this->listen_sock_fd, conn_handler = move(conn_handler)]() {
            int accept_retval = ::accept4(fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
            conn_handler(
                accept_retval >= 0
                    ? make_ok_result(shared_fd(accept_retval))
                    : make_raw_error_result_from_errno());
        });
}


socket_connections_fd_acceptor::~socket_connections_fd_acceptor()
{
    listen_sock_watch_handle->disable();
}

}


shared_ptr<interface_type> make_full_socket_connections_fd_acceptor(
    fd_watcher &watcher, owned_fd &&listen_sock_fd, result_handler<shared_fd> &&conn_handler)
{
    return make_shared<socket_connections_fd_acceptor>(watcher, move(listen_sock_fd), move(conn_handler));
}


shared_ptr<interface_type> make_socket_connections_fd_acceptor(
    fd_watcher &watcher, owned_fd &&listen_sock_fd, function<void(shared_fd)> &&conn_handler)
{
    return make_full_socket_connections_fd_acceptor(
        watcher, move(listen_sock_fd),
        [conn_handler = move(conn_handler)](auto conn_res) {
            if (conn_res.is_ok()) {
                conn_handler(move(conn_res.get_value()));
            }
        });
}

}
