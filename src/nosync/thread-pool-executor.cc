// This file is part of libnosync library. See LICENSE file for license details.
#include <exception>
#include <memory>
#include <nosync/synchronized-queue.h>
#include <nosync/thread-pool-executor.h>
#include <thread>
#include <utility>
#include <vector>

using std::current_exception;
using std::exception_ptr;
using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::thread;
using std::vector;


namespace nosync
{

namespace
{

class thread_pool_executor
{
public:
    explicit thread_pool_executor(
        thread_pool_executor_destroy_mode destroy_mode,
        function<void(exception_ptr)> &&exception_handler);
    ~thread_pool_executor();

    thread_pool_executor(const thread_pool_executor &) = delete;
    thread_pool_executor &operator=(const thread_pool_executor &) = delete;

    thread_pool_executor(thread_pool_executor &&) = delete;
    thread_pool_executor &operator=(thread_pool_executor &&) = delete;

    void add_worker_threads(unsigned thread_count);
    void enqueue_task(function<void()> &&task);

private:
    static void run_worker_thread(
        synchronized_queue<function<void()>> &tasks_queue,
        function<void(exception_ptr)> &&exception_handler);

    thread_pool_executor_destroy_mode destroy_mode;
    shared_ptr<function<void(exception_ptr)>> exception_handler;
    shared_ptr<synchronized_queue<function<void()>>> tasks_queue;
    vector<thread> worker_threads;
};


thread_pool_executor::thread_pool_executor(
    thread_pool_executor_destroy_mode destroy_mode,
    function<void(exception_ptr)> &&exception_handler)
    : destroy_mode(destroy_mode),
    exception_handler(make_shared<function<void(exception_ptr)>>(move(exception_handler))),
    tasks_queue(make_shared<synchronized_queue<function<void()>>>()), worker_threads()
{
}


thread_pool_executor::~thread_pool_executor()
{
    for (unsigned i = 0; i < worker_threads.size(); ++i) {
        tasks_queue->enqueue(nullptr);
    }

    for (auto &t : worker_threads) {
        if (destroy_mode == thread_pool_executor_destroy_mode::join) {
            t.join();
        } else {
            t.detach();
        }
    }
}


void thread_pool_executor::add_worker_threads(unsigned thread_count)
{
    exception_ptr threads_init_exception;
    for (unsigned i = 0; i < thread_count; ++i) {
        worker_threads.emplace_back(
            [tasks_queue = tasks_queue, exception_handler = exception_handler]() mutable {
                run_worker_thread(*tasks_queue, move(*exception_handler));
            });
    }
}


void thread_pool_executor::enqueue_task(function<void()> &&task)
{
    if (task) {
        tasks_queue->enqueue(move(task));
    }
}


void thread_pool_executor::run_worker_thread(
    synchronized_queue<function<void()>> &tasks_queue,
    function<void(exception_ptr)> &&exception_handler)
{
    while (true) {
        function<void()> task = tasks_queue.dequeue();
        if (!task) {
            break;
        }

        try {
            task();
        } catch (...) {
            exception_handler(current_exception());
        }
    }
}

}


function<void(function<void()>)> make_thread_pool_executor(
    unsigned thread_count, thread_pool_executor_destroy_mode destroy_mode,
    function<void(exception_ptr)> &&exception_handler)
{
    auto executor = make_shared<thread_pool_executor>(destroy_mode, move(exception_handler));
    executor->add_worker_threads(thread_count);

    return [executor = move(executor)](auto task) {
        executor->enqueue_task(move(task));
    };
}

}
