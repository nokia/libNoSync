// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__PERIODIC_TICK_GENERATOR_H
#define NOSYNC__PERIODIC_TICK_GENERATOR_H

#include <chrono>
#include <functional>
#include <nosync/event-loop.h>
#include <nosync/interface-type.h>
#include <memory>


namespace nosync
{

std::shared_ptr<interface_type> make_periodic_tick_generator(
    event_loop &evloop, std::chrono::nanoseconds interval, std::function<void()> &&tick_func);

}

#endif /* NOSYNC__PERIODIC_TICK_GENERATOR_H */
