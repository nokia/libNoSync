// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/net-utils.h>
#include <nosync/result-utils.h>
#include <nosync/socket-datagrams-acceptor.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <utility>

using std::function;
using std::get;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::tuple;
using std::unique_ptr;


namespace nosync
{

namespace
{

class socket_datagrams_acceptor : public interface_type
{
public:
    socket_datagrams_acceptor(
        fd_watcher &watcher, owned_fd &&sock_fd, size_t max_datagram_size,
        result_handler<tuple<unique_ptr<socket_address>, string>> &&datagrams_handler);
    ~socket_datagrams_acceptor() override;

private:
    owned_fd sock_fd;
    unique_ptr<activity_handle> sock_watch_handle;
};


socket_datagrams_acceptor::socket_datagrams_acceptor(
    fd_watcher &watcher, owned_fd &&sock_fd, size_t max_datagram_size,
    result_handler<tuple<unique_ptr<socket_address>, string>> &&datagrams_handler)
    : sock_fd(move(sock_fd))
{
    sock_watch_handle = watcher.add_watch(
        *this->sock_fd, fd_watch_mode::input,
        [fd = *this->sock_fd, datagrams_handler = move(datagrams_handler), max_datagram_size]() {
            datagrams_handler(receive_datagram_via_socket(fd, max_datagram_size));
        });
}


socket_datagrams_acceptor::~socket_datagrams_acceptor()
{
    sock_watch_handle->disable();
}

}


shared_ptr<interface_type> make_full_socket_datagrams_acceptor(
    fd_watcher &watcher, owned_fd &&sock_fd, size_t max_datagram_size,
    result_handler<tuple<unique_ptr<socket_address>, string>> &&datagrams_handler)
{
    return make_shared<socket_datagrams_acceptor>(watcher, move(sock_fd), max_datagram_size, move(datagrams_handler));
}


shared_ptr<interface_type> make_socket_datagrams_acceptor(
    fd_watcher &watcher, owned_fd &&sock_fd, size_t max_datagram_size,
    function<void(unique_ptr<socket_address>, string)> &&datagrams_handler)
{
    return make_full_socket_datagrams_acceptor(
        watcher, move(sock_fd), max_datagram_size,
        [datagrams_handler = move(datagrams_handler)](auto datagram_res) {
            if (datagram_res.is_ok()) {
                auto &datagram = datagram_res.get_value();
                datagrams_handler(
                    move(get<unique_ptr<socket_address>>(datagram)),
                    move(get<string>(datagram)));
            }
        });
}

}
