// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/async-funcs-proxy.h>

using nosync::async_funcs_proxy;
using std::make_shared;
using std::make_unique;


TEST(NosyncFuncsProxy, WrapCallDestroyCall)
{
    auto funcs_proxy = make_unique<async_funcs_proxy>();

    auto counter = make_shared<unsigned>(0);
    auto inc_counter_func = funcs_proxy->wrap(
        [counter]() {
            ++*counter;
        });

    ASSERT_GT(counter.use_count(), 1);

    ASSERT_EQ(*counter, 0U);
    inc_counter_func();
    ASSERT_EQ(*counter, 1U);
    inc_counter_func();
    ASSERT_EQ(*counter, 2U);

    funcs_proxy.reset();
    ASSERT_EQ(counter.use_count(), 1);

    inc_counter_func();
    ASSERT_EQ(*counter, 2U);
    inc_counter_func();
    ASSERT_EQ(*counter, 2U);
}


TEST(NosyncFuncsProxy, WrapCopyCallDestroyCall)
{
    auto funcs_proxy = make_unique<async_funcs_proxy>();

    auto counter = make_shared<unsigned>(0);
    auto inc_counter_func_1 = funcs_proxy->wrap(
        [counter]() {
            ++*counter;
        });
    auto inc_counter_func_2 = inc_counter_func_1;

    ASSERT_GT(counter.use_count(), 1);

    ASSERT_EQ(*counter, 0U);
    inc_counter_func_1();
    inc_counter_func_2();
    ASSERT_EQ(*counter, 2U);

    funcs_proxy.reset();
    ASSERT_EQ(counter.use_count(), 1);

    inc_counter_func_1();
    inc_counter_func_2();
    ASSERT_EQ(*counter, 2U);
}
