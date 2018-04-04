// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SOCKET_ACTIVATION_H
#define NOSYNC__SOCKET_ACTIVATION_H

#include <nosync/owned-fd.h>
#include <vector>


namespace nosync
{

std::vector<owned_fd> takeover_socket_activated_fds();

}

#endif /* NOSYNC__SOCKET_ACTIVATION_H */
