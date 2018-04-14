#include <experimental/optional>
#include <nosync/const-bytes-reader.h>
#include <nosync/event-loop.h>
#include <nosync/fd-bytes-writer.h>
#include <nosync/func-bytes-writer.h>
#include <nosync/net-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/reader-writer-bytes-io.h>
#include <nosync/result-handler-utils.h>
#include <nosync/socket-datagrams-acceptor.h>
#include <nosync/socket-datagrams-responding-acceptor.h>
#include <system_error>
#include <utility>

namespace ch = std::chrono;
using std::errc;
using std::error_code;
using std::experimental::nullopt;
using std::experimental::optional;
using std::function;
using std::get;
using std::make_error_code;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::unique_ptr;


namespace nosync
{

namespace
{

class failed_recv_bytes_io : public bytes_io
{
public:
    failed_recv_bytes_io(event_loop &evloop, error_code pending_error);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

    void write_bytes(string &&data, result_handler<void> &&res_handler) override;

private:
    event_loop &evloop;
    optional<error_code> pending_error;
};


failed_recv_bytes_io::failed_recv_bytes_io(event_loop &evloop, error_code pending_error)
    : evloop(evloop), pending_error(pending_error)
{
}


void failed_recv_bytes_io::read_some_bytes(
    size_t, ch::nanoseconds, result_handler<string> &&res_handler)
{
    auto err = pending_error.value_or(make_error_code(errc::not_connected));
    if (pending_error) {
        pending_error = nullopt;
    }

    invoke_result_handler_later(evloop, move(res_handler), raw_error_result(err));
}


void failed_recv_bytes_io::write_bytes(string &&, result_handler<void> &&res_handler)
{
    invoke_result_handler_later(evloop, move(res_handler), raw_error_result(errc::not_connected));
}


shared_ptr<bytes_writer> make_datagram_writer(fd_watching_event_loop &evloop, shared_ptr<socket_address> &&dest_address)
{
    return make_func_bytes_writer(
        [&evloop, dest_address = move(dest_address)](auto data, auto res_handler) {
            auto sock_fd_res = open_connected_datagram_socket(dest_address->get_view());
            if (sock_fd_res.is_ok()) {
                auto writer = make_fd_bytes_writer(evloop, move(sock_fd_res.get_value()));
                writer->write_bytes(move(data), move(res_handler));
            } else {
                invoke_result_handler_later(evloop, move(res_handler), raw_error_result(sock_fd_res));
            }
        });
}

}


shared_ptr<interface_type> make_socket_datagrams_responding_acceptor(
    fd_watching_event_loop &evloop, owned_fd &&sock_fd, size_t max_request_datagram_size,
    function<void(shared_ptr<bytes_io> &&)> &&datagrams_processor)
{
    return make_full_socket_datagrams_acceptor(
        evloop, move(sock_fd), max_request_datagram_size,
        [&evloop, datagrams_processor = move(datagrams_processor)](auto recv_res) {
            datagrams_processor(
                recv_res.is_ok()
                    ? make_reader_writer_bytes_io(
                        make_const_bytes_reader(evloop, move(get<string>(recv_res.get_value()))),
                        make_datagram_writer(evloop, move(get<unique_ptr<socket_address>>(recv_res.get_value()))))
                    : make_shared<failed_recv_bytes_io>(evloop, recv_res.get_error()));
        });
}

}
