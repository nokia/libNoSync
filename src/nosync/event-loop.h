// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EVENT_LOOP_H
#define NOSYNC__EVENT_LOOP_H

#include <chrono>
#include <functional>
#include <nosync/activity-handle.h>
#include <nosync/eclock.h>
#include <nosync/interface-type.h>
#include <memory>


namespace nosync
{

/*!
Base interface for event loop implementations.

Event loop object allows executing tasks asynchronously.

An event loop implementation must:
 - run code in iterations with one task executed in each iteration,
 - run tasks sequentially and within single thread,
 - provide information about "event loop time" via get_etime function call
which returns:
   a) the same value for all calls within single iteration,
   b) non-decreasing values for calls in consecutive iterations,
 - provide support for executing tasks in future iterations via invoke_at
function call,
 - execute each task enqueued with invoke_at call not earlier then than value
of "time" argument passed with the call (get_etime called from within the task
must return value not less than specified there),
 - execute tasks enqueued with the same "time" value in the order of enqueuing
them.

A task passed to event loop via invoke_at must not:
 - use blocking calls (such as i/o operations in blocking mode),
 - perform CPU-intensive tasks (such as calculations on big data sets).

In other words, it must return very quickly.
*/
class event_loop : public interface_type
{
public:
    virtual std::unique_ptr<activity_handle> invoke_at(std::chrono::time_point<eclock> time, std::function<void()> &&task) = 0;
    virtual std::chrono::time_point<eclock> get_etime() const = 0;
};

}

#endif /* NOSYNC__EVENT_LOOP_H */
