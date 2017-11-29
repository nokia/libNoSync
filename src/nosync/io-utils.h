// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__IO_UTILS_H
#define NOSYNC__IO_UTILS_H

#include <array>
#include <experimental/string_view>
#include <nosync/owned-fd.h>
#include <nosync/result.h>
#include <string>


namespace nosync
{

owned_fd open_abstract_listening_stream_socket(const std::string &abstract_path, int conn_backlog_size);

std::array<owned_fd, 2> create_nonblocking_pipe();

}

#endif /* NOSYNC__IO_UTILS_H */
