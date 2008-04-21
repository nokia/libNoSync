// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__NET_UTILS_H
#define NOSYNC__NET_UTILS_H

#include <cstdint>
#include <memory>
#include <nosync/owned-fd.h>
#include <nosync/result.h>
#include <nosync/socket-address.h>
#include <string>
#include <tuple>


struct sockaddr;


namespace nosync
{

int get_local_socket_domain();

result<std::unique_ptr<socket_address>> make_local_abstract_socket_address(const std::string &abstract_path);
result<std::unique_ptr<socket_address>> make_local_filesystem_socket_address(const std::string &path);

std::unique_ptr<socket_address> make_ipv4_socket_address(std::uint32_t host_addr, std::uint16_t port);
std::unique_ptr<socket_address> make_ipv4_localhost_socket_address(std::uint16_t port);

result<owned_fd> open_stream_socket(int domain);
result<owned_fd> open_datagram_socket(int domain);

result<owned_fd> open_connected_datagram_socket(socket_address_view addr);
result<owned_fd> open_bound_datagram_socket(socket_address_view addr);

result<owned_fd> open_connected_stream_socket(socket_address_view addr);
result<owned_fd> open_listening_stream_socket(socket_address_view addr, int conn_backlog_size);

result<std::tuple<std::unique_ptr<socket_address>, std::string>> receive_datagram_via_socket(int sock_fd, std::size_t max_data_size);

bool is_stream_socket(int sock_fd);
bool is_datagram_socket(int sock_fd);

result<int> get_socket_int_option(int sock_fd, int level, int opt_name);

result<std::string> bind_local_socket_to_auto_abstract_path(int sock_fd);

result<void> connect_local_socket_to_abstract_path(int sock_fd, const std::string &abstract_path);

result<owned_fd> open_local_abstract_connected_stream_socket(const std::string &abstract_path);
result<owned_fd> open_local_abstract_listening_stream_socket(const std::string &abstract_path, int conn_backlog_size);

}

#endif /* NOSYNC__NET_UTILS_H */
