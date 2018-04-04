// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/activity-handle-mock.h>

using nosync::activity_handle_mock;
using std::make_unique;


TEST(NosyncActivityHandle, CreateMockInstance)
{
    ASSERT_TRUE(make_unique<activity_handle_mock>());
}
