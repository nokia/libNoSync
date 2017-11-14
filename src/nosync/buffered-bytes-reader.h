// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BUFFERED_BYTES_READER_H
#define NOSYNC__BUFFERED_BYTES_READER_H

#include <cstddef>
#include <nosync/bytes-reader.h>
#include <memory>


namespace nosync
{

/*!
Create bytes_reader decorator which adds input data buffering.

This function should be used to improve performance when the user reads data in
small portions (e.g. one byte at a time) and per-read overhead in the underlying
bytes_reader implementation is relatively big.

Buffering is implemented by synchronous prefetching of data from the underlying
bytes_reader, storing extra data in the internal buffer and serving consecutive
requests from the buffer if possible.

The generated object obeys general contract of bytes_reader but its behaviour in
case of multiple requests running at the same time may not be what one expects
due to data prefetching. In such case it's recommendeed to use it together
with bytes_reader implementation which ensures handling read requests
sequentially (queuing them if necessary).
*/
std::shared_ptr<bytes_reader> make_buffered_bytes_reader(
    std::size_t buffer_size, std::shared_ptr<bytes_reader> &&base_reader);

}

#endif /* NOSYNC__BUFFERED_BYTES_READER_H */
