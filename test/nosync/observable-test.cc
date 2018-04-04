// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/observable-mock.h>

using nosync::observable_mock;
using std::make_unique;


TEST(NosyncObservable, CreateMockInstance)
{
    make_unique<observable_mock>();
}
