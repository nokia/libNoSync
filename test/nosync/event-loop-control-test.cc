// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/event-loop-control-mock.h>

using nosync::event_loop_control_mock;
using std::make_unique;


TEST(NosyncEventLoopControl, CreateMockInstance)
{
    ASSERT_TRUE(make_unique<event_loop_control_mock>());
}
