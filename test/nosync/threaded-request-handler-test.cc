// This file is part of libnosync library. See LICENSE file for license details.
#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <nosync/event-loop-based-mt-executor.h>
#include <nosync/event-loop-utils.h>
#include <nosync/event-loop.h>
#include <nosync/ppoll-based-event-loop.h>
#include <nosync/request-handler.h>
#include <nosync/result-utils.h>
#include <nosync/thread-pool-executor.h>
#include <nosync/threaded-request-handler.h>
#include <nosync/type-utils.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace std::string_literals;
using nosync::result;
using nosync::make_copy;
using nosync::make_timeout_raw_error_result;
using nosync::make_ok_result;
using nosync::make_event_loop_based_mt_executor;
using nosync::make_ppoll_based_event_loop;
using nosync::make_thread_pool_executor;
using nosync::make_threaded_request_handler;
using std::errc;
using std::runtime_error;
using std::size_t;
using std::string;
using std::thread;
using std::vector;


TEST(NosyncThreadedRequestHandler, CheckThreadIds)
{
    auto evloop = make_ppoll_based_event_loop();

    auto evloop_mt_executor = make_event_loop_based_mt_executor(*evloop);
    auto req_handler = make_threaded_request_handler<string, thread::id>(
        make_copy(evloop_mt_executor), nosync::make_thread_pool_executor(1),
        [](auto, auto) {
            return make_ok_result(std::this_thread::get_id());
        });

    vector<thread::id> saved_result_handler_thread_ids;
    vector<result<thread::id>> saved_results;

    invoke_later(
        *evloop,
        [&]() {
            req_handler->handle_request(
                ""s, 1ns,
                [&](auto result) {
                    saved_result_handler_thread_ids.push_back(std::this_thread::get_id());
                    saved_results.push_back(result);
                    invoke_later(
                        *evloop,
                        [&]() {
                            req_handler = nullptr;
                            evloop_mt_executor = nullptr;
                        });
                });
        });

    evloop->run_iterations();

    ASSERT_EQ(saved_result_handler_thread_ids.size(), 1U);
    ASSERT_EQ(saved_result_handler_thread_ids.front(), std::this_thread::get_id());
    ASSERT_EQ(saved_results.size(), 1U);
    ASSERT_TRUE(saved_results.front().is_ok());
    ASSERT_NE(saved_results.front().get_value(), std::this_thread::get_id());
}


TEST(NosyncThreadedRequestHandler, CheckHandlerResults)
{
    constexpr auto tick_time = 4ns;
    constexpr auto calculation_time = tick_time * 5;
    const auto test_string = "test string"s;

    auto evloop = make_ppoll_based_event_loop();

    auto evloop_mt_executor = make_event_loop_based_mt_executor(*evloop);
    auto strsize_calculator = make_threaded_request_handler<string, size_t>(
        make_copy(evloop_mt_executor), make_thread_pool_executor(1),
        [calculation_time](auto request, auto timeout) {
            if (timeout < calculation_time) {
                return make_timeout_raw_error_result().as_result<size_t>();
            }

            std::this_thread::sleep_for(calculation_time);
            return make_ok_result(request.size());
        });

    vector<result<size_t>> saved_results;

    for (auto timeout : {calculation_time - tick_time, calculation_time}) {
        invoke_later(
            *evloop,
            [strsize_calculator, test_string = test_string, timeout, &saved_results]() mutable {
                strsize_calculator->handle_request(
                    move(test_string), timeout,
                    [strsize_calculator, &saved_results](auto result) {
                        saved_results.push_back(result);
                    });
            });
    }

    invoke_later(
        *evloop,
        [&]() {
            strsize_calculator = nullptr;
            evloop_mt_executor = nullptr;
        });

    evloop->run_iterations();

    ASSERT_EQ(saved_results.size(), 2U);
    ASSERT_EQ(saved_results[0], make_timeout_raw_error_result().as_result<size_t>());
    ASSERT_EQ(saved_results[1], make_ok_result(test_string.size()));
}


TEST(NosyncThreadedRequestHandler, CheckHandlerException)
{
    auto evloop = make_ppoll_based_event_loop();

    auto evloop_mt_executor = make_event_loop_based_mt_executor(*evloop);
    auto req_handler = make_threaded_request_handler<string, string>(
        make_copy(evloop_mt_executor), make_thread_pool_executor(1),
        [](auto, auto) -> result<string> {
            throw runtime_error("handler error");
        });

    vector<result<string>> saved_results;

    invoke_later(
        *evloop,
        [&]() {
            req_handler->handle_request(
                ""s, 1ns,
                [&](auto result) {
                    saved_results.push_back(result);
                    invoke_later(
                        *evloop,
                        [&]() {
                            req_handler = nullptr;
                            evloop_mt_executor = nullptr;
                        });
                });
        });

    evloop->run_iterations();

    ASSERT_EQ(saved_results.size(), 1U);
    ASSERT_FALSE(saved_results.front().is_ok());
    ASSERT_EQ(saved_results.front().get_error(), make_error_code(errc::owner_dead));
}
