// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <deque>
#include <experimental/optional>
#include <experimental/string_view>
#include <nosync/eclock.h>
#include <nosync/input-messages-dispatch-handler.h>
#include <nosync/memory-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/requests-queue.h>
#include <nosync/result-utils.h>
#include <nosync/time-utils.h>
#include <nosync/utils.h>
#include <system_error>
#include <tuple>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using std::deque;
using std::enable_shared_from_this;
using std::errc;
using std::experimental::nullopt;
using std::experimental::optional;
using std::experimental::string_view;
using std::function;
using std::get;
using std::make_shared;
using std::move;
using std::nullptr_t;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;


namespace nosync
{

namespace
{

class input_messages_dispatch_handler : public request_handler<string, string>, public enable_shared_from_this<input_messages_dispatch_handler>
{
public:
    input_messages_dispatch_handler(
        event_loop &evloop, shared_ptr<request_handler<nullptr_t, string>> &&messages_reader,
        function<optional<string>(string_view)> &&message_id_decoder);
    ~input_messages_dispatch_handler() override;

    void handle_request(
        string &&msg_id, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    void read_next_message_if_needed();
    void handle_pending_requests_timeouts();
    optional<result_handler<string>> try_pull_matching_pending_handler(string_view message);

    event_loop &evloop;
    shared_ptr<request_handler<nullptr_t, string>> messages_reader;
    function<optional<string>(string_view)> message_id_decoder;
    bool read_ongoing;
    deque<tuple<string, ch::time_point<eclock>, result_handler<string>>> pending_requests;
    requests_queue<string, string> new_pending_requests;
};


input_messages_dispatch_handler::input_messages_dispatch_handler(
    event_loop &evloop, shared_ptr<request_handler<nullptr_t, string>> &&messages_reader,
    function<optional<string>(string_view)> &&message_id_decoder)
    : evloop(evloop), messages_reader(move(messages_reader)), message_id_decoder(move(message_id_decoder)),
    read_ongoing(false), pending_requests(), new_pending_requests(evloop)
{
}


input_messages_dispatch_handler::~input_messages_dispatch_handler()
{
    if (!pending_requests.empty()) {
        invoke_later(
            evloop,
            [cancel_requests = move(pending_requests)]() mutable {
                for (auto &req : cancel_requests) {
                    auto &res_handler = get<result_handler<string>>(req);
                    res_handler(raw_error_result(errc::operation_canceled));
                    res_handler = nullptr;
                }
            });
    }
}


void input_messages_dispatch_handler::handle_request(
    string &&msg_id, ch::nanoseconds timeout, result_handler<string> &&res_handler)
{
    if (!read_ongoing) {
        pending_requests.emplace_back(
            move(msg_id), time_point_sat_add(evloop.get_etime(), timeout), move(res_handler));
        read_next_message_if_needed();
    } else {
        new_pending_requests.push_request(
            move(msg_id), time_point_sat_add(evloop.get_etime(), timeout), move(res_handler));
    }
}


void input_messages_dispatch_handler::read_next_message_if_needed()
{
    if (pending_requests.empty() || read_ongoing) {
        return;
    }

    const auto min_timeout_end = min_transformed_value(
        pending_requests.begin(), pending_requests.end(),
        [&](const auto &req) {
            return get<ch::time_point<eclock>>(req);
        });

    messages_reader->handle_request(
        nullptr,
        std::max(min_timeout_end - evloop.get_etime(), 0ns),
        [disp_handler_wptr = weak_from_that(this)](auto read_res) {
            auto disp_handler_ptr = disp_handler_wptr.lock();
            if (!disp_handler_ptr) {
                return;
            }

            disp_handler_ptr->read_ongoing = false;

            while (disp_handler_ptr->new_pending_requests.has_requests()) {
                disp_handler_ptr->pending_requests.emplace_back(
                    disp_handler_ptr->new_pending_requests.pull_next_request());
            }

            optional<result_handler<string>> res_handler;
            if (read_res.is_ok()) {
                res_handler = disp_handler_ptr->try_pull_matching_pending_handler(read_res.get_value());
            } else {
                if (!disp_handler_ptr->pending_requests.empty() && read_res.get_error() != make_error_code(errc::timed_out)) {
                    res_handler = move(get<result_handler<string>>(disp_handler_ptr->pending_requests.front()));
                    disp_handler_ptr->pending_requests.pop_front();
                }
            }

            if (res_handler) {
                (*res_handler)(read_res);
            }

            disp_handler_ptr->handle_pending_requests_timeouts();

            disp_handler_ptr->read_next_message_if_needed();
        });
    read_ongoing = true;
}


void input_messages_dispatch_handler::handle_pending_requests_timeouts()
{
    auto timeouted_reqs_end_iter = std::stable_partition(
        pending_requests.begin(), pending_requests.end(),
        [now = evloop.get_etime()](const auto &req) {
            const auto req_timeout_end = get<ch::time_point<eclock>>(req);
            return req_timeout_end <= now;
        });

    vector<result_handler<string>> timeouted_res_handlers;
    std::for_each(
        pending_requests.begin(), timeouted_reqs_end_iter,
        [&](auto &req) {
            timeouted_res_handlers.emplace_back(
                move(get<result_handler<string>>(req)));
        });

    pending_requests.erase(pending_requests.begin(), timeouted_reqs_end_iter);

    for (auto &res_handler : timeouted_res_handlers) {
        res_handler(make_timeout_raw_error_result());
        res_handler = nullptr;
    }
}


optional<result_handler<string>> input_messages_dispatch_handler::try_pull_matching_pending_handler(string_view message)
{
    auto msg_id = message_id_decoder(message);
    if (!msg_id) {
        return nullopt;
    }

    auto matching_req_iter = std::find_if(
        pending_requests.begin(), pending_requests.end(),
        [&](const auto &req) {
            return get<string>(req) == *msg_id;
        });

    optional<result_handler<string>> matching_handler;
    if (matching_req_iter != pending_requests.end()) {
        matching_handler = move(get<result_handler<string>>(*matching_req_iter));
        pending_requests.erase(matching_req_iter);
    }

    return matching_handler;
}

}


shared_ptr<request_handler<string, string>> make_input_messages_dispatch_handler(
    event_loop &evloop, shared_ptr<request_handler<nullptr_t, string>> &&messages_reader,
    function<optional<string>(string_view)> &&message_id_decoder)
{
    return make_shared<input_messages_dispatch_handler>(
        evloop, move(messages_reader), move(message_id_decoder));
}

}
