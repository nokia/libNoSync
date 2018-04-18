// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SOCKET_ADDRESS_H
#define NOSYNC__SOCKET_ADDRESS_H

#include <cstddef>


struct sockaddr;


namespace nosync
{

struct socket_address_view
{
    const ::sockaddr *addr;
    std::size_t addr_size;
};


class socket_address
{
public:
    virtual ~socket_address() = default;

    virtual int get_address_family() const = 0;
    virtual socket_address_view get_view() const = 0;

protected:
    socket_address() = default;
    socket_address(const socket_address &) = default;
    socket_address(socket_address &&) = default;
};

}

#endif /* NOSYNC__SOCKET_ADDRESS_H */
