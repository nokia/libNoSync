// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-reader-mock.h>

using nosync::bytes_reader_mock;
using std::make_unique;


TEST(NosyncBytesReader, CreateMockInstance)
{
    make_unique<bytes_reader_mock>();
}
