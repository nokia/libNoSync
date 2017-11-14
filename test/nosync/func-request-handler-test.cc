// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <nosync/func-request-handler.h>
#include <nosync/result.h>
#include <nosync/type-utils.h>
#include <tuple>
#include <utility>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using nosync::make_func_request_handler;
using nosync::make_ok_result;
using nosync::result;
using nosync::make_copy;
using std::function;
using std::get;
using std::move;
using std::tuple;
using std::vector;


TEST(NosyncFuncRequestHandler, TestMultipleRequests)
{
    constexpr auto requests_count = 3U;
    constexpr auto test_timeout = 100ms;

    vector<tuple<unsigned, ch::nanoseconds, function<void(result<char>)>>> saved_requests;

    auto req_handler = make_func_request_handler<unsigned, char>(
        [&saved_requests](auto req, auto timeout, auto res_handler) {
            saved_requests.emplace_back(req, timeout, move(res_handler));
        });

    ASSERT_EQ(saved_requests.size(), 0U);

    vector<result<char>> saved_results;

    for (unsigned i = 0; i < requests_count; ++i) {
        req_handler->handle_request(
            make_copy(i), test_timeout,
            [&saved_results](auto result) {
                saved_results.push_back(move(result));
            });
    }

    ASSERT_EQ(saved_requests.size(), 3U);
    ASSERT_EQ(saved_results.size(), 0U);

    for (unsigned i = 0; i < requests_count; ++i) {
        ASSERT_EQ(get<unsigned>(saved_requests[i]), i);
        ASSERT_EQ(get<ch::nanoseconds>(saved_requests[i]), test_timeout);
        ASSERT_TRUE(get<function<void(result<char>)>>(saved_requests[i]));

        const auto test_result = make_ok_result<char>('A' + i);
        get<function<void(result<char>)>>(saved_requests[i])(test_result);
        ASSERT_EQ(saved_results.size(), i + 1);
        ASSERT_EQ(saved_results.back(), test_result);
    }
}
