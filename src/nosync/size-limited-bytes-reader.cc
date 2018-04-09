// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <chrono>
#include <memory>
#include <nosync/bytes-reader-utils.h>
#include <nosync/eclock.h>
#include <nosync/memory-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/requests-queue.h>
#include <nosync/size-limited-bytes-reader.h>
#include <nosync/time-utils.h>
#include <nosync/type-utils.h>
#include <system_error>
#include <utility>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using namespace std::string_literals;
using std::errc;
using std::enable_shared_from_this;
using std::get;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

class size_limited_bytes_reader : public bytes_reader, public enable_shared_from_this<size_limited_bytes_reader>
{
public:
    size_limited_bytes_reader(event_loop &evloop, size_t total_size_limit, shared_ptr<bytes_reader> &&base_reader);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    void handle_next_pending_request();

    event_loop &evloop;
    shared_ptr<bytes_reader> base_reader;
    requests_queue<size_t, string> pending_requests;
    size_t remaining_size_limit;
    bool read_ongoing;
};


size_limited_bytes_reader::size_limited_bytes_reader(
    event_loop &evloop, size_t total_size_limit, shared_ptr<bytes_reader> &&base_reader)
    : evloop(evloop), base_reader(move(base_reader)), pending_requests(evloop), remaining_size_limit(total_size_limit),
    read_ongoing(false)
{
}


void size_limited_bytes_reader::read_some_bytes(
    size_t max_size, ch::nanoseconds timeout,
    result_handler<string> &&res_handler)
{
    if (max_size == 0) {
        invoke_result_handler_later_via_bytes_reader(
            *base_reader, move(res_handler), make_ok_result(""s));
        return;
    }
    if (remaining_size_limit == 0) {
        invoke_result_handler_later_via_bytes_reader(
            *base_reader, move(res_handler), raw_error_result(errc::file_too_large));
        return;
    }

    if (!read_ongoing) {
        base_reader->read_some_bytes(
            std::min(max_size, remaining_size_limit), timeout,
            [reader_wptr = weak_from_that(this), res_handler = move(res_handler)](auto read_res) {
                const size_t read_bytes_count = read_res.is_ok() ? read_res.get_value().size() : 0U;
                res_handler(move(read_res));

                auto reader_ptr = reader_wptr.lock();
                if (reader_ptr) {
                    reader_ptr->read_ongoing = false;
                    reader_ptr->remaining_size_limit -= read_bytes_count;
                    reader_ptr->handle_next_pending_request();
                }
            });

        read_ongoing = true;
    } else {
        pending_requests.push_request(
            make_copy(max_size), time_point_sat_add(evloop.get_etime(), timeout), move(res_handler));
    }
}


void size_limited_bytes_reader::handle_next_pending_request()
{
    if (!pending_requests.has_requests()) {
        return;
    }

    auto req = pending_requests.pull_next_request();

    read_some_bytes(
        get<size_t>(req), std::max(get<ch::time_point<eclock>>(req) - evloop.get_etime(), 0ns),
        move(get<result_handler<string>>(req)));
}

}


shared_ptr<bytes_reader> make_size_limited_bytes_reader(
    event_loop &evloop, size_t total_size_limit, shared_ptr<bytes_reader> &&base_reader)
{
    return make_shared<size_limited_bytes_reader>(evloop, total_size_limit, move(base_reader));
}

}
