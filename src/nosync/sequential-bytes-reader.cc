// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <deque>
#include <experimental/optional>
#include <functional>
#include <nosync/activity-handle.h>
#include <nosync/eclock.h>
#include <nosync/event-loop-utils.h>
#include <nosync/memory-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/result-utils.h>
#include <nosync/sequential-bytes-reader.h>
#include <nosync/time-utils.h>
#include <nosync/utils.h>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using std::deque;
using std::enable_shared_from_this;
using std::errc;
using std::experimental::nullopt;
using std::experimental::optional;
using std::get;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::tuple;
using std::unique_ptr;


namespace nosync
{

namespace
{

class sequential_bytes_reader : public bytes_reader, public enable_shared_from_this<sequential_bytes_reader>
{
public:
    sequential_bytes_reader(event_loop &evloop, shared_ptr<bytes_reader> &&base_reader);
    ~sequential_bytes_reader() override;

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    void reschedule_timeout_task();
    void start_next_pending_read();
    void handle_pending_timeouts();

    event_loop &evloop;
    shared_ptr<bytes_reader> base_reader;
    bool read_ongoing;
    deque<tuple<size_t, ch::time_point<eclock>, result_handler<string>>> pending_requests;
    optional<tuple<ch::time_point<eclock>, unique_ptr<activity_handle>>> scheduled_timeout_task;
};


sequential_bytes_reader::sequential_bytes_reader(
    event_loop &evloop, shared_ptr<bytes_reader> &&base_reader)
    : evloop(evloop), base_reader(move(base_reader)), read_ongoing(false),
    pending_requests(), scheduled_timeout_task()
{
}


sequential_bytes_reader::~sequential_bytes_reader()
{
    if (scheduled_timeout_task) {
        get<unique_ptr<activity_handle>>(*scheduled_timeout_task)->disable();
    }

    for (auto &req : pending_requests) {
        get<result_handler<string>>(req)(
            raw_error_result(errc::operation_canceled));
    }
}


void sequential_bytes_reader::read_some_bytes(
    size_t max_size, ch::nanoseconds timeout,
    result_handler<string> &&res_handler)
{
    if (read_ongoing) {
        pending_requests.emplace_back(max_size, time_point_sat_add(evloop.get_etime(), timeout), move(res_handler));
        reschedule_timeout_task();
        return;
    }

    read_ongoing = true;

    base_reader->read_some_bytes(
        max_size, timeout,
        [reader_wptr = weak_from_that(this), res_handler = move(res_handler)](auto read_res) {
            res_handler(move(read_res));

            auto reader_ptr = reader_wptr.lock();
            if (reader_ptr) {
                if (!reader_ptr->pending_requests.empty()) {
                    reader_ptr->start_next_pending_read();
                } else {
                    reader_ptr->read_ongoing = false;
                }
            }
        });
}


void sequential_bytes_reader::start_next_pending_read()
{
    auto req = move(pending_requests.front());
    pending_requests.pop_front();

    read_some_bytes(
        get<size_t>(req),
        get<ch::time_point<eclock>>(req) - evloop.get_etime(),
        move(get<result_handler<string>>(req)));
}


void sequential_bytes_reader::reschedule_timeout_task()
{
    if (pending_requests.empty()) {
        if (scheduled_timeout_task) {
            get<unique_ptr<activity_handle>>(*scheduled_timeout_task)->disable();
            scheduled_timeout_task = nullopt;
        }

        return;
    }

    auto min_timeout_end = min_transformed_value(
        pending_requests.cbegin(), pending_requests.cend(),
        [&](const auto &elem) {
            return get<ch::time_point<eclock>>(elem);
        });

    if (!scheduled_timeout_task || get<ch::time_point<eclock>>(*scheduled_timeout_task) != min_timeout_end) {
        if (scheduled_timeout_task) {
            get<unique_ptr<activity_handle>>(*scheduled_timeout_task)->disable();
        }

        auto timeout_task_handle = evloop.invoke_at(
            min_timeout_end,
            make_weak_this_func_proxy(
                this,
                [](auto &self) {
                    self.handle_pending_timeouts();
                }));
        scheduled_timeout_task.emplace(min_timeout_end, move(timeout_task_handle));
    }
}


void sequential_bytes_reader::handle_pending_timeouts()
{
    const auto now = evloop.get_etime();
    for (auto req : pending_requests) {
        if (get<ch::time_point<eclock>>(req) <= now) {
            auto &req_res_handler = get<result_handler<string>>(req);
            req_res_handler(make_timeout_raw_error_result());
            req_res_handler = nullptr;
        }
    }

    pending_requests.erase(
        std::remove_if(
            pending_requests.begin(), pending_requests.end(),
            [](auto &req) {
                return !get<result_handler<string>>(req);
            }),
        pending_requests.end());
}

}


shared_ptr<bytes_reader> make_sequential_bytes_reader(
    event_loop &evloop, shared_ptr<bytes_reader> &&base_reader)
{
    return make_shared<sequential_bytes_reader>(evloop, move(base_reader));
}

}
