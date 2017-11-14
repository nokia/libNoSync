// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/memory-utils.h>

using nosync::weak_from_that;
using std::enable_shared_from_this;
using std::make_shared;


namespace
{

struct test_shared_type : public enable_shared_from_this<test_shared_type>
{
};

}


TEST(NosyncMemoryUtils, WeakFromThisCheckEqual)
{
    auto test_obj = make_shared<test_shared_type>();
    auto weak_obj = weak_from_that(test_obj.get());
    ASSERT_EQ(weak_obj.lock(), test_obj);
}
