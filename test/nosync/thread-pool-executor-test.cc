// This file is part of libnosync library. See LICENSE file for license details.
#include <atomic>
#include <gtest/gtest.h>
#include <mutex>
#include <nosync/thread-pool-executor.h>
#include <stdexcept>
#include <thread>
#include <vector>

using nosync::make_thread_pool_executor;
using nosync::thread_pool_executor_destroy_mode;
using std::atomic_uint;
using std::exception_ptr;
using std::function;
using std::lock_guard;
using std::mutex;
using std::rethrow_exception;
using std::runtime_error;
using std::thread;
using std::vector;


TEST(NosyncThreadPoolExecutor, Init)
{
    auto executor = make_thread_pool_executor(1);
}


TEST(NosyncThreadPoolExecutor, SimpleTask)
{
    atomic_uint counter(0);

    {
        auto executor = make_thread_pool_executor(1);

        executor(
            [&counter]() {
                ++counter;
            });
    }

    ASSERT_EQ(counter, 1U);
}


TEST(NosyncThreadPoolExecutor, NullTask)
{
    atomic_uint counter(0);

    {
        auto executor = make_thread_pool_executor(1);

        executor(function<void()>());

        executor(
            [&counter]() {
                ++counter;
            });
    }

    ASSERT_EQ(counter, 1U);
}


TEST(NosyncThreadPoolExecutor, ThreadIds)
{
    constexpr auto tasks_count = 4;

    const auto main_thread_id = std::this_thread::get_id();

    vector<unsigned> tasks_numbers;
    vector<thread::id> tasks_thread_ids;
    mutex tasks_mutex;

    {
        auto executor = make_thread_pool_executor(1);

        for (unsigned i = 0; i < tasks_count; ++i) {
            executor(
                [i, &tasks_numbers, &tasks_thread_ids, &tasks_mutex]() {
                    lock_guard<mutex> lock(tasks_mutex);
                    tasks_numbers.push_back(i);
                    tasks_thread_ids.push_back(std::this_thread::get_id());
                });
        }
    }

    lock_guard<mutex> lock(tasks_mutex);

    ASSERT_EQ(tasks_numbers.size(), tasks_count);
    ASSERT_EQ(tasks_thread_ids.size(), tasks_count);

    const auto executor_thread_id = tasks_thread_ids.front();
    ASSERT_NE(executor_thread_id, main_thread_id);

    for (unsigned i = 0; i < tasks_numbers.size(); ++i) {
        ASSERT_EQ(tasks_numbers[i], i);
        ASSERT_EQ(tasks_thread_ids[i], executor_thread_id);
    }
}


TEST(NosyncThreadPoolExecutor, ExceptionHandler)
{
    atomic_uint counter(0);
    atomic_uint exception_counter(0);

    {
        auto executor = make_thread_pool_executor(
            1, thread_pool_executor_destroy_mode::join,
            [&exception_counter](exception_ptr eptr) {
                try {
                    rethrow_exception(eptr);
                } catch (runtime_error &) {
                    ++exception_counter;
                }
            });

        executor(
            [&counter]() {
                ++counter;
                throw runtime_error("");
            });
    }

    ASSERT_EQ(counter, 1U);
    ASSERT_EQ(exception_counter, 1U);
}
