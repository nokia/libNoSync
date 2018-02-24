// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__OWNED_FD_H
#define NOSYNC__OWNED_FD_H


namespace nosync
{

class owned_fd
{
public:
    owned_fd() noexcept;
    explicit owned_fd(int fd) noexcept;
    ~owned_fd();

    owned_fd(owned_fd &&other) noexcept;
    owned_fd &operator=(owned_fd &&other) noexcept;

    void reset() noexcept;
    int release() noexcept;

    explicit operator bool() const noexcept;

    int operator*() const noexcept;

    owned_fd(const owned_fd &other) = delete;
    void operator=(const owned_fd &other) = delete;

private:
    int fd;
};

}

#endif /* NOSYNC__OWNED_FD_H */
