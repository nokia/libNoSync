// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__ASYNC_FUNCS_PROXY_H
#define NOSYNC__ASYNC_FUNCS_PROXY_H

#include <functional>
#include <memory>
#include <vector>


namespace nosync
{

class async_funcs_proxy
{
public:
    async_funcs_proxy() = default;

    async_funcs_proxy(const async_funcs_proxy &) = delete;
    async_funcs_proxy &operator=(const async_funcs_proxy &) = delete;

    std::function<void()> wrap(std::function<void()> &&func);

private:
    std::shared_ptr<std::vector<std::shared_ptr<std::function<void()>>>> active_funcs_registry;
};

}

#endif /* NOSYNC__ASYNC_FUNCS_PROXY_H */
