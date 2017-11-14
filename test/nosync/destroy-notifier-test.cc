// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/destroy-notifier.h>
#include <nosync/interface-type.h>

using nosync::interface_type;
using nosync::make_destroy_notifier;
using nosync::make_shared_destroy_notifier;
using std::shared_ptr;
using std::unique_ptr;


TEST(NosyncDestroyNotifier, CheckDestroyCall)
{
    auto call_counter = 0U;

    unique_ptr<interface_type> notifier = make_destroy_notifier(
        [&call_counter]() {
            ++call_counter;
        });
    ASSERT_EQ(call_counter, 0U);

    notifier.reset();
    ASSERT_EQ(call_counter, 1U);
}


TEST(NosyncDestroyNotifier, CheckSharedDestroyCall)
{
    auto call_counter = 0U;

    shared_ptr<interface_type> notifier = make_shared_destroy_notifier(
        [&call_counter]() {
            ++call_counter;
        });
    ASSERT_EQ(call_counter, 0U);

    notifier.reset();
    ASSERT_EQ(call_counter, 1U);
}
