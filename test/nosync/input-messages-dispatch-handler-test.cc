// This file is part of libnosync library. See LICENSE file for license details.
#include <cstddef>
#include <deque>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/func-request-handler.h>
#include <nosync/input-messages-dispatch-handler.h>
#include <nosync/manual-event-loop.h>
#include <nosync/result-utils.h>
#include <nosync/result.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::manual_event_loop;
using nosync::make_func_request_handler;
using nosync::make_input_messages_dispatch_handler;
using nosync::make_ok_result;
using nosync::make_timeout_raw_error_result;
using nosync::request_handler;
using nosync::result;
using std::deque;
using std::experimental::nullopt;
using std::function;
using std::make_tuple;
using std::move;
using std::nullptr_t;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::tuple;
using std::vector;


namespace
{

constexpr auto test_req_timeout = 10ns;


template<typename Req, typename Res>
void repeat_request_handler_calls(
    shared_ptr<request_handler<Req, Res>> req_handler,
    unsigned repeat_count,
    function<void(request_handler<Req, Res> &, function<void(result<string>)>)> req_caller,
    function<void(result<string>)> res_handler)
{
    if (repeat_count != 0) {
        req_caller(
            *req_handler,
            [req_handler, repeat_count, req_caller, res_handler = move(res_handler)](auto res) {
                res_handler(move(res));
                repeat_request_handler_calls(req_handler, repeat_count - 1, move(req_caller), move(res_handler));
            });
    }
}


template<typename T>
function<void(T)> make_vector_pusher(vector<T> &out_vector)
{
    return [&out_vector](auto value) {
        out_vector.push_back(move(value));
    };
}

}


TEST(NosyncInputMessagesDispatchHandler, MultipleRequests)
{
    auto evloop = manual_event_loop::create();

    deque<string> input_msgs = {"1abc"s, "2de"s, "3fghi"s, "4jklmn"s};
    auto msgs_reader = make_func_request_handler<nullptr_t, string>(
        [&evloop, &input_msgs](auto, auto timeout, auto res_handler) {
            if (!input_msgs.empty()) {
                auto msg = input_msgs.front();
                input_msgs.pop_front();
                evloop->invoke_at(
                    evloop->get_etime() + 1ns,
                    [&input_msgs, res_handler = move(res_handler), msg = move(msg)]() {
                        res_handler(make_ok_result(msg));
                    });
            } else {
                evloop->invoke_at(
                    evloop->get_etime() + timeout,
                    [&input_msgs, res_handler = move(res_handler)]() {
                        res_handler(make_timeout_raw_error_result().as_result<string>());
                    });
            }
        });

    auto messages_dispatcher = make_input_messages_dispatch_handler(
        *evloop, move(msgs_reader),
        [](auto msg) {
            return !msg.empty() ? std::experimental::make_optional(msg.substr(0, 1).to_string()) : nullopt;
        });

    vector<tuple<unsigned, result<string>>> saved_results;
    for (unsigned id : {5, 2, 4, 1}) {
        messages_dispatcher->handle_request(
            to_string(id), test_req_timeout,
            [id, &saved_results](auto res) {
                saved_results.emplace_back(id, move(res));
            });
    }

    evloop->process_time_passage(0ns);
    for (unsigned i = 0; i < 10; ++i) {
        evloop->process_time_passage(1ns);
        while (evloop->get_earliest_task_time() == std::experimental::make_optional(evloop->get_etime())) {
            evloop->process_time_passage(0ns);
        }
    }

    ASSERT_EQ(saved_results.size(), 4U);
    ASSERT_EQ(saved_results[0], make_tuple(1U, make_ok_result("1abc"s)));
    ASSERT_EQ(saved_results[1], make_tuple(2U, make_ok_result("2de"s)));
    ASSERT_EQ(saved_results[2], make_tuple(4U, make_ok_result("4jklmn"s)));
    ASSERT_EQ(saved_results[3], make_tuple(5U, make_timeout_raw_error_result().as_result<string>()));
}
