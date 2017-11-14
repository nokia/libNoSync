// This file is part of libnosync library. See LICENSE file for license details.
#include <gtest/gtest.h>
#include <memory>
#include <nosync/bytes-writer-mock.h>

using nosync::bytes_writer_mock;
using std::make_unique;


TEST(NosyncBytesWriter, CreateMockInstance)
{
    make_unique<bytes_writer_mock>();
}
