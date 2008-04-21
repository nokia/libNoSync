// This file is part of libnosync library. See LICENSE file for license details.
#include <cstddef>
#include <cstring>
#include <endian.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <nosync/net-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/result-utils.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <system_error>
#include <utility>

using std::errc;
using std::error_code;
using std::make_error_code;
using std::make_unique;
using std::move;
using std::size_t;
using std::string;
using std::tuple;
using std::uint16_t;
using std::uint32_t;
using std::unique_ptr;


namespace nosync
{

namespace
{

class un_socket_address : public socket_address
{
public:
    un_socket_address(const ::sockaddr_un &addr, size_t addr_size);

    un_socket_address(const un_socket_address &) = default;
    un_socket_address(un_socket_address &&) = default;

    int get_address_family() const override;
    socket_address_view get_view() const override;

private:
    ::sockaddr_un addr;
    size_t addr_size;
};


class in_socket_address : public socket_address
{
public:
    explicit in_socket_address(const ::sockaddr_in &addr);

    in_socket_address(const in_socket_address &) = default;
    in_socket_address(in_socket_address &&) = default;

    int get_address_family() const override;
    socket_address_view get_view() const override;

private:
    ::sockaddr_in addr;
};


class any_socket_address : public socket_address
{
public:
    any_socket_address(const ::sockaddr_storage &addr, size_t addr_size);

    any_socket_address(const any_socket_address &) = default;
    any_socket_address(any_socket_address &&) = default;

    int get_address_family() const override;
    socket_address_view get_view() const override;

private:
    ::sockaddr_storage addr;
    size_t addr_size;
};


un_socket_address::un_socket_address(const ::sockaddr_un &addr, size_t addr_size)
    : addr(addr), addr_size(addr_size)
{
}


int un_socket_address::get_address_family() const
{
    return AF_UNIX;
}


socket_address_view un_socket_address::get_view() const
{
    return {reinterpret_cast<const ::sockaddr *>(&addr), addr_size};
}


in_socket_address::in_socket_address(const ::sockaddr_in &addr)
    : addr(addr)
{
}


int in_socket_address::get_address_family() const
{
    return AF_INET;
}


socket_address_view in_socket_address::get_view() const
{
    return {reinterpret_cast<const ::sockaddr *>(&addr), sizeof(addr)};
}




any_socket_address::any_socket_address(const ::sockaddr_storage &addr, size_t addr_size)
    : addr(addr), addr_size(addr_size)
{
}


int any_socket_address::get_address_family() const
{
    return addr.ss_family;
}


socket_address_view any_socket_address::get_view() const
{
    return {reinterpret_cast<const ::sockaddr *>(&addr), addr_size};
}


un_socket_address make_local_socket_unnamed_address()
{
    ::sockaddr_un addr;
    addr.sun_family = AF_UNIX;

    const auto addr_size = offsetof(decltype(addr), sun_path);

    return un_socket_address(addr, addr_size);
}


result<string> get_local_socket_raw_path(int sock_fd)
{
    ::sockaddr_un addr;
    ::socklen_t addr_size = sizeof(addr);
    if (::getsockname(sock_fd, reinterpret_cast<::sockaddr *>(&addr), &addr_size) != 0) {
        return make_raw_error_result_from_errno();
    }

    constexpr auto addr_sun_path_offset = offsetof(decltype(addr), sun_path);

    bool local_addr_valid =
        addr.sun_family == AF_UNIX
        && addr_size <= sizeof(addr)
        && addr_size >= addr_sun_path_offset;

    return local_addr_valid
        ? make_ok_result(string(&addr.sun_path[0], addr_size - addr_sun_path_offset))
        : raw_error_result(errc::address_family_not_supported);
}


result<string> get_local_socket_abstract_path(int sock_fd)
{
    auto path_res = get_local_socket_raw_path(sock_fd);
    if (!path_res.is_ok()) {
        return path_res;
    }

    auto &path = path_res.get_value();

    return !path.empty() && path.front() == '\0'
        ? make_ok_result(path.substr(1))
        : raw_error_result(errc::bad_address);
}


result<owned_fd> open_socket(int domain, int type)
{
    constexpr auto sock_protocol = 0;
    int socket_retval = ::socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, sock_protocol);

    return socket_retval >= 0
        ? make_ok_result(owned_fd(socket_retval))
        : make_raw_error_result_from_errno();
}


result<void> connect_socket(int sock_fd, socket_address_view addr)
{
    int connect_retval = ::connect(sock_fd, addr.addr, addr.addr_size);

    return connect_retval == 0
        ? make_ok_result()
        : make_raw_error_result_from_errno();
}


result<void> listen_on_socket(int sock_fd, int backlog_size)
{
    int listen_retval = ::listen(sock_fd, backlog_size);

    return listen_retval == 0
        ? make_ok_result()
        : make_raw_error_result_from_errno();
}


result<void> bind_socket(int sock_fd, socket_address_view addr)
{
    int bind_retval = ::bind(sock_fd, addr.addr, addr.addr_size);

    return bind_retval == 0
        ? make_ok_result()
        : make_raw_error_result_from_errno();
}


result<int> get_socket_type(int sock_fd)
{
    return get_socket_int_option(sock_fd, SOL_SOCKET, SO_TYPE);
}

}


int get_local_socket_domain()
{
    return AF_UNIX;
}


result<unique_ptr<socket_address>> make_local_abstract_socket_address(const string &abstract_path)
{
    ::sockaddr_un addr;
    if (abstract_path.size() > sizeof(addr.sun_path) - 1) {
        return raw_error_result(errc::filename_too_long);
    }

    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0';
    abstract_path.copy(addr.sun_path + 1, abstract_path.size());

    const auto addr_size = offsetof(decltype(addr), sun_path) + 1 + abstract_path.size();

    return make_ok_result(make_unique<un_socket_address>(addr, addr_size));
}


result<unique_ptr<socket_address>> make_local_filesystem_socket_address(const string &path)
{
    ::sockaddr_un addr;

    error_code path_error;
    if (path.size() >= sizeof(addr.sun_path)) {
        path_error = make_error_code(errc::filename_too_long);
    } else if (path.empty() || path.front() == '\0' || path.find('\0') != string::npos) {
        path_error = make_error_code(errc::bad_address);
    }

    if (path_error) {
        return raw_error_result(path_error);
    }

    addr.sun_family = AF_UNIX;
    path.copy(addr.sun_path, path.size());
    addr.sun_path[path.size()] = '\0';

    const auto addr_size = offsetof(decltype(addr), sun_path) + path.size() + 1;

    return make_ok_result(make_unique<un_socket_address>(addr, addr_size));
}


unique_ptr<socket_address> make_ipv4_socket_address(uint32_t host_addr, uint16_t port)
{
    const ::sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = static_cast<in_port_t>(htobe16(port)),
        .sin_addr = {
            .s_addr = htobe32(host_addr),
        },
        .sin_zero = {},
    };

    return make_unique<in_socket_address>(addr);
}


unique_ptr<socket_address> make_ipv4_localhost_socket_address(uint16_t port)
{
    return make_ipv4_socket_address(INADDR_LOOPBACK, port);
}


result<owned_fd> open_stream_socket(int domain)
{
    return open_socket(domain, SOCK_STREAM);
}


result<owned_fd> open_datagram_socket(int domain)
{
    return open_socket(domain, SOCK_DGRAM);
}


result<owned_fd> open_connected_datagram_socket(socket_address_view addr)
{
    auto sock_res = open_socket(addr.addr->sa_family, SOCK_DGRAM);
    if (!sock_res.is_ok()) {
        return sock_res;
    }

    auto connect_res = connect_socket(*sock_res.get_value(), addr);

    return connect_res.is_ok()
        ? move(sock_res)
        : raw_error_result(connect_res);
}


result<owned_fd> open_bound_datagram_socket(socket_address_view addr)
{
    auto sock_res = open_socket(addr.addr->sa_family, SOCK_DGRAM);
    if (!sock_res.is_ok()) {
        return sock_res;
    }

    auto bind_res = bind_socket(*sock_res.get_value(), addr);

    return bind_res.is_ok()
        ? move(sock_res)
        : raw_error_result(bind_res);
}


result<owned_fd> open_connected_stream_socket(socket_address_view addr)
{
    auto sock_fd_res = open_stream_socket(addr.addr->sa_family);
    if (!sock_fd_res.is_ok()) {
        return sock_fd_res;
    }

    const auto &sock_fd = sock_fd_res.get_value();

    auto connect_res = connect_socket(*sock_fd, addr);

    return connect_res.is_ok()
        ? move(sock_fd_res)
        : raw_error_result(connect_res);
}


result<owned_fd> open_listening_stream_socket(socket_address_view addr, int conn_backlog_size)
{
    auto sock_fd_res = open_stream_socket(addr.addr->sa_family);
    if (!sock_fd_res.is_ok()) {
        return sock_fd_res;
    }

    const auto &sock_fd = sock_fd_res.get_value();

    auto bind_res = bind_socket(*sock_fd, addr);
    auto listen_res = bind_res.is_ok()
        ? listen_on_socket(*sock_fd, conn_backlog_size)
        : raw_error_result(bind_res);

    return listen_res.is_ok()
        ? move(sock_fd_res)
        : raw_error_result(listen_res);
}


result<tuple<unique_ptr<socket_address>, string>> receive_datagram_via_socket(int sock_fd, size_t max_data_size)
{
    ::sockaddr_storage src_addr;
    ::socklen_t src_addr_size = sizeof(src_addr);
    auto data_buf = make_unique<char[]>(max_data_size + 1);

    ssize_t recv_retval = ::recvfrom(
        sock_fd, data_buf.get(), max_data_size + 1, 0, reinterpret_cast<sockaddr *>(&src_addr), &src_addr_size);
    if (recv_retval < 0) {
        return make_raw_error_result_from_errno();
    }

    if (src_addr_size > sizeof(src_addr)) {
        return raw_error_result(errc::bad_address);
    }

    const auto data_size = static_cast<size_t>(recv_retval);
    if (data_size > max_data_size) {
        return raw_error_result(errc::message_size);
    }

    return make_ok_result(
        make_tuple(
            make_unique<any_socket_address>(src_addr, src_addr_size),
            string(data_buf.get(), data_size)));
}


bool is_stream_socket(int sock_fd)
{
    auto sock_type = get_socket_type(sock_fd);
    return sock_type.is_ok() && sock_type.get_value() == SOCK_STREAM;
}


bool is_datagram_socket(int sock_fd)
{
    auto sock_type = get_socket_type(sock_fd);
    return sock_type.is_ok() && sock_type.get_value() == SOCK_DGRAM;
}


result<int> get_socket_int_option(int sock_fd, int level, int opt_name)
{
    int opt_value;
    ::socklen_t opt_value_size = sizeof(opt_value);
    int sockopt_retval = ::getsockopt(sock_fd, level, opt_name, &opt_value, &opt_value_size);

    return sockopt_retval == 0
        ? opt_value_size == sizeof(opt_value)
            ? make_ok_result(opt_value)
            : raw_error_result(errc::message_size)
        : make_raw_error_result_from_errno();
}


result<string> bind_local_socket_to_auto_abstract_path(int sock_fd)
{
    auto bind_res = bind_socket(sock_fd, make_local_socket_unnamed_address().get_view());

    return bind_res.is_ok()
        ? get_local_socket_abstract_path(sock_fd)
        : raw_error_result(bind_res);
}


result<void> connect_local_socket_to_abstract_path(int sock_fd, const string &abstract_path)
{
    auto addr_res = make_local_abstract_socket_address(abstract_path);

    return addr_res.is_ok()
        ? connect_socket(sock_fd, addr_res.get_value()->get_view())
        : raw_error_result(addr_res);
}


result<owned_fd> open_local_abstract_connected_stream_socket(const string &abstract_path)
{
    auto addr_res = make_local_abstract_socket_address(abstract_path);
    return addr_res.is_ok()
        ? open_connected_stream_socket(addr_res.get_value()->get_view())
        : raw_error_result(addr_res);
}


result<owned_fd> open_local_abstract_listening_stream_socket(const string &abstract_path, int conn_backlog_size)
{
    auto addr_res = make_local_abstract_socket_address(abstract_path);
    return addr_res.is_ok()
        ? open_listening_stream_socket(addr_res.get_value()->get_view(), conn_backlog_size)
        : raw_error_result(addr_res);
}

}
