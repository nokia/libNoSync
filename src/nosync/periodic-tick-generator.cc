// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/async-funcs-proxy.h>
#include <nosync/eclock.h>
#include <nosync/periodic-tick-generator.h>
#include <utility>

namespace ch = std::chrono;
using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;


namespace nosync
{

class periodic_tick_generator : public interface_type
{
public:
    periodic_tick_generator(event_loop &evloop, ch::nanoseconds interval, function<void()> &&tick_func);

private:
    void schedule_next_tick();

    event_loop &evloop;
    ch::nanoseconds interval;
    function<void()> tick_func;
    ch::time_point<eclock> last_tick_time;
    async_funcs_proxy asyncs_proxy;
};


periodic_tick_generator::periodic_tick_generator(event_loop &evloop, ch::nanoseconds interval, function<void()> &&tick_func)
    : evloop(evloop), interval(interval), tick_func(move(tick_func)), last_tick_time(evloop.get_etime())
{
    schedule_next_tick();
}


void periodic_tick_generator::schedule_next_tick()
{
    evloop.invoke_at(
        last_tick_time + interval,
        asyncs_proxy.wrap(
            [this]() {
                last_tick_time += interval;
                tick_func();
                schedule_next_tick();
            }));
}


shared_ptr<interface_type> make_periodic_tick_generator(
    event_loop &evloop, ch::nanoseconds interval, function<void()> &&tick_func)
{
    return make_shared<periodic_tick_generator>(evloop, interval, move(tick_func));
}

}
