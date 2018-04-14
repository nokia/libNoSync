#include <nosync/fd-watch-utils.h>
#include <nosync/func-request-handler.h>
#include <nosync/net-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/result-handler-utils.h>
#include <nosync/result-utils.h>
#include <nosync/socket-connections-fd-requester.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>

namespace ch = std::chrono;
using nosync::result_handler;
using std::error_code;
using std::generic_category;
using std::move;
using std::shared_ptr;


namespace nosync
{

namespace
{

void handle_delayed_connect(
    fd_watching_event_loop &evloop, shared_fd sock, ch::nanoseconds timeout,
    result_handler<shared_fd> &&res_handler)
{
    int sock_fd_no = *sock;
    watch_fd_with_timeout(
        evloop, sock_fd_no, fd_watch_mode::output, timeout,
        transform_result_handler<void>(
            move(res_handler),
            [sock = move(sock)]() mutable {
                auto sock_err_res = get_socket_int_option(*sock, SOL_SOCKET, SO_ERROR);
                return sock_err_res.is_ok()
                    ? sock_err_res.get_value() == 0
                        ? make_ok_result(move(sock))
                        : raw_error_result(error_code(sock_err_res.get_value(), generic_category()))
                    : raw_error_result(sock_err_res);
            }));
}

}


void request_socket_connection_fd(
    fd_watching_event_loop &evloop, const shared_ptr<socket_address> &addr,
    ch::nanoseconds timeout, result_handler<shared_fd> &&res_handler)
{
    result<shared_fd> sock_res = open_stream_socket(addr->get_address_family());
    if (!sock_res.is_ok()) {
        invoke_result_handler_later(evloop, move(res_handler), raw_error_result(sock_res));
        return;
    }

    auto &sock = sock_res.get_value();

    auto addr_view = addr->get_view();
    int connect_retval = ::connect(*sock, addr_view.addr, addr_view.addr_size);

    if (connect_retval == 0) {
        invoke_result_handler_later(evloop, move(res_handler), make_ok_result(move(sock)));
    } else {
        handle_delayed_connect(evloop, move(sock), timeout, move(res_handler));
    }
}


shared_ptr<request_handler<shared_ptr<socket_address>, shared_fd>> make_socket_connections_fd_requester(
    fd_watching_event_loop &evloop)
{
    return make_func_request_handler<shared_ptr<socket_address>, shared_fd>(
        [&evloop](auto sock_addr, auto timeout, auto res_handler) {
            request_socket_connection_fd(evloop, move(sock_addr), timeout, move(res_handler));
        });
}

}
