// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__THREAD_POOL_EXECUTOR_H
#define NOSYNC__THREAD_POOL_EXECUTOR_H

#include <exception>
#include <functional>
#include <nosync/noop-func.h>


namespace nosync
{

enum class thread_pool_executor_destroy_mode
{
    join,
    detach,
};


std::function<void(std::function<void()>)> make_thread_pool_executor(
    unsigned thread_count, thread_pool_executor_destroy_mode destroy_mode = thread_pool_executor_destroy_mode::join,
    std::function<void(std::exception_ptr)> &&exception_handler = noop_func());

}

#endif /* NOSYNC__THREAD_POOL_EXECUTOR_H */
