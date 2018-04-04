// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__MANUAL_EVENT_LOOP_H
#define NOSYNC__MANUAL_EVENT_LOOP_H

#include <chrono>
#include <cstdint>
#include <experimental/optional>
#include <functional>
#include <nosync/eclock.h>
#include <nosync/event-loop.h>
#include <map>
#include <memory>
#include <tuple>


namespace nosync
{

class manual_event_loop_task_handle;


class manual_event_loop : public event_loop
{
public:
    static std::shared_ptr<manual_event_loop> create(std::chrono::time_point<eclock> init_time = {});

    manual_event_loop(std::chrono::time_point<eclock> init_time);

    std::experimental::optional<std::chrono::time_point<eclock>> get_earliest_task_time() const;
    bool process_time_passage(std::chrono::nanoseconds time_delta);
    void quit();

    bool is_enabled(const manual_event_loop_task_handle &task_handle) const;
    void disable(const manual_event_loop_task_handle &task_handle);

    std::unique_ptr<activity_handle> invoke_at(std::chrono::time_point<eclock> time, std::function<void()> &&task) override;
    std::chrono::time_point<eclock> get_etime() const override;

private:
    std::chrono::time_point<eclock> last_event_time;
    std::map<std::tuple<std::chrono::time_point<eclock>, std::uint64_t>, std::function<void()>> pending_tasks;
    std::uint64_t next_task_id;
    bool quit_request_pending;
};

}

#endif /* NOSYNC__MANUAL_EVENT_LOOP_H */
