// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/request-handler-mock.h>

using nosync::request_handler_mock;
using std::make_unique;


TEST(NosyncRequestHandler, CreateMockInstance)
{
    make_unique<request_handler_mock<int, char>>();
}
