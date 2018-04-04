// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-io-mock.h>

using nosync::bytes_io_mock;
using std::make_unique;


TEST(NosyncBytesIo, CreateMockInstance)
{
    make_unique<bytes_io_mock>();
}
