// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/full-fd-watching-event-loop-mock.h>

using nosync::full_fd_watching_event_loop_mock;
using std::make_unique;


TEST(NosyncFullFdWatchingEventLoop, CreateMockInstance)
{
    make_unique<full_fd_watching_event_loop_mock>();
}
