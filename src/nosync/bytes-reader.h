// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_READER_H
#define NOSYNC__BYTES_READER_H

#include <chrono>
#include <cstddef>
#include <functional>
#include <nosync/interface-type.h>
#include <nosync/result-handler.h>
#include <string>


namespace nosync
{

/*!
Interface for reading bytes asynchronously with time limit.

The interface allows reading bytes asynchronously with time limit specified
by caller for each operation. The result is always provided asynchronously
(as result<...> object), even if the data is immediately available.

Like with Unix read() function, the interface lets the caller specify maximum
number of bytes to read but the actual number of bytes returned may be smaller.

Also like with read() function, zero bytes may be returned only if either
 - zero was provided as the maximum read size,
 - end of file/stream was reached.

If max_size is zero then the implementation must return empty result without
any additional delays.

If specified timeout is zero then the implementation must return only bytes
which are immediately available. If no data is immediately available then it
must return without any additional delays with:
 - timeout error if max_size is non-zero,
 - empty result if max_size is zero.

On timeout the returned result object will hold errc::timed_out error, as if
it was created with std::make_error_code(std::errc::timed_out).
*/
class bytes_reader : public interface_type
{
public:
    virtual void read_some_bytes(
        std::size_t max_size, std::chrono::nanoseconds timeout,
        result_handler<std::string> &&res_handler) = 0;
};

}

#endif /* NOSYNC__BYTES_READER_H */
