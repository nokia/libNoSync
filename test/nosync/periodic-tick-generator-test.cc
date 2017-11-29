// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/event-loop.h>
#include <nosync/manual-event-loop.h>
#include <nosync/periodic-tick-generator.h>

using namespace std::chrono_literals;
using nosync::make_periodic_tick_generator;
using nosync::manual_event_loop;


TEST(NosyncTickGenerator, CreateTickTickDestroy)
{
    auto evloop = manual_event_loop::create();
    ASSERT_FALSE(evloop->get_earliest_task_time());

    auto tick_counter = 0U;

    {
        auto pps_tick_gen = make_periodic_tick_generator(
            *evloop, 1s,
            [&]() {
                ++tick_counter;
            });

        ASSERT_EQ(tick_counter, 0U);
        ASSERT_TRUE(evloop->get_earliest_task_time());

        evloop->process_time_passage(500ms);
        ASSERT_EQ(tick_counter, 0U);
        ASSERT_TRUE(evloop->get_earliest_task_time());

        evloop->process_time_passage(500ms);
        ASSERT_EQ(tick_counter, 1U);
        ASSERT_TRUE(evloop->get_earliest_task_time());

        evloop->process_time_passage(1s);
        ASSERT_EQ(tick_counter, 2U);
        ASSERT_TRUE(evloop->get_earliest_task_time());

        evloop->process_time_passage(3s);
        ASSERT_EQ(tick_counter, 5U);
        ASSERT_TRUE(evloop->get_earliest_task_time());
    }

    evloop->process_time_passage(2s);
    ASSERT_EQ(tick_counter, 5U);
}
