#include <nosync/fd-bytes-io.h>
#include <nosync/func-request-handler.h>
#include <nosync/socket-connections-bytes-io-requester.h>
#include <nosync/socket-connections-fd-requester.h>
#include <utility>

namespace ch = std::chrono;
using std::move;
using std::shared_ptr;


namespace nosync
{

void request_socket_connection_bytes_io(
    fd_watching_event_loop &evloop, const shared_ptr<socket_address> &addr,
    ch::nanoseconds timeout, result_handler<shared_ptr<bytes_io>> &&res_handler)
{
    request_socket_connection_fd(
        evloop, addr, timeout,
        transform_result_handler<shared_fd>(
            move(res_handler),
            [&evloop](auto sock) {
                return make_ok_result(make_fd_bytes_io(evloop, move(sock)));
            }));
}


shared_ptr<request_handler<shared_ptr<socket_address>, shared_ptr<bytes_io>>> make_socket_connections_bytes_io_requester(
    fd_watching_event_loop &evloop)
{
    return make_func_request_handler<shared_ptr<socket_address>, shared_ptr<bytes_io>>(
        [&evloop](auto addr, auto timeout, auto res_handler) {
            request_socket_connection_bytes_io(evloop, move(addr), timeout, move(res_handler));
        });
}

}
