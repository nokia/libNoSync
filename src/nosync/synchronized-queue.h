// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__SYNCHRONIZED_QUEUE_H
#define NOSYNC__SYNCHRONIZED_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>


namespace nosync
{

template<typename T>
class synchronized_queue
{
public:
    synchronized_queue();

    void enqueue(T element);
    T dequeue();

private:
    std::queue<T> elements;
    std::mutex elements_mutex;
    std::condition_variable elements_present_cv;
};

}

#include <nosync/synchronized-queue-impl.h>

#endif /* NOSYNC__SYNCHRONIZED_QUEUE_H */
