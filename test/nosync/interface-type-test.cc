// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/interface-type-mock.h>

using nosync::interface_type_mock;
using std::make_unique;


TEST(NosyncInterfaceType, CreateMockInstance)
{
    ASSERT_TRUE(make_unique<interface_type_mock>());
}
