// This file is part of libnosync library. See LICENSE file for license details.
#include <map>
#include <nosync/manual-event-loop.h>
#include <stdexcept>
#include <utility>

namespace ch = std::chrono;
using std::experimental::nullopt;
using std::experimental::optional;
using std::function;
using std::get;
using std::make_shared;
using std::make_tuple;
using std::make_unique;
using std::move;
using std::runtime_error;
using std::shared_ptr;
using std::uint64_t;
using std::unique_ptr;


namespace nosync
{

class manual_event_loop_task_handle : public activity_handle
{
public:
    manual_event_loop_task_handle(manual_event_loop &evloop, ch::time_point<eclock> time, uint64_t id);

    bool is_enabled() const override;
    void disable() override;

    manual_event_loop &evloop;
    const ch::time_point<eclock> time;
    const uint64_t id;
};


manual_event_loop_task_handle::manual_event_loop_task_handle(
    manual_event_loop &evloop, ch::time_point<eclock> time, uint64_t id)
    : evloop(evloop), time(time), id(id)
{
}


bool manual_event_loop_task_handle::is_enabled() const
{
    return evloop.is_enabled(*this);
}


void manual_event_loop_task_handle::disable()
{
    return evloop.disable(*this);
}


shared_ptr<manual_event_loop> manual_event_loop::create(ch::time_point<eclock> init_time)
{
    return make_shared<manual_event_loop>(init_time);
}


manual_event_loop::manual_event_loop(ch::time_point<eclock> init_time)
    : last_event_time(init_time), pending_tasks(), next_task_id(0U), quit_request_pending(false)
{
}


optional<ch::time_point<eclock>> manual_event_loop::get_earliest_task_time() const
{
    if (pending_tasks.empty()) {
        return nullopt;
    }

    return get<ch::time_point<eclock>>(pending_tasks.begin()->first);
}


bool manual_event_loop::process_time_passage(ch::nanoseconds time_delta)
{
    last_event_time += time_delta;

    while (!pending_tasks.empty() && get<ch::time_point<eclock>>(pending_tasks.cbegin()->first) <= last_event_time) {
        auto head_iter = pending_tasks.begin();
        auto task = move(head_iter->second);
        pending_tasks.erase(head_iter);
        task();

        if (quit_request_pending) {
            quit_request_pending = false;
            break;
        }
    }

    return pending_tasks.empty() || get<ch::time_point<eclock>>(pending_tasks.cbegin()->first) > last_event_time;
}


void manual_event_loop::quit()
{
    quit_request_pending = true;
}


bool manual_event_loop::is_enabled(const manual_event_loop_task_handle &task_handle) const
{
    auto task_iter = pending_tasks.find(make_tuple(task_handle.time, task_handle.id));
    return task_iter != pending_tasks.end();
}


void manual_event_loop::disable(const manual_event_loop_task_handle &task_handle)
{
    auto task_iter = pending_tasks.find(make_tuple(task_handle.time, task_handle.id));
    if (task_iter == pending_tasks.end()) {
        throw runtime_error("got cancel for non-existing task");
    }

    pending_tasks.erase(task_iter);
}


unique_ptr<activity_handle> manual_event_loop::invoke_at(ch::time_point<eclock> time, function<void()> &&task)
{
    auto task_id = next_task_id;
    ++next_task_id;
    pending_tasks.emplace(make_tuple(time, task_id), move(task));

    return make_unique<manual_event_loop_task_handle>(*this, time, task_id);
}


ch::time_point<eclock> manual_event_loop::get_etime() const
{
    return last_event_time;
}

}
