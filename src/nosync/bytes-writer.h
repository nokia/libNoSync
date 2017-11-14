// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_WRITER_H
#define NOSYNC__BYTES_WRITER_H

#include <functional>
#include <nosync/interface-type.h>
#include <nosync/result-handler.h>
#include <string>


namespace nosync
{

/*!
Interface for writing bytes asynchronously.

The interface allows writing all specified bytes with the result always
provided asynchronously (as result<void> object), even if it's immediately
available. Success is reported only if all bytes were written successfully.
*/
class bytes_writer : public interface_type
{
public:
    virtual void write_bytes(std::string &&data, result_handler<void> &&res_handler) = 0;
};

}

#endif /* NOSYNC__BYTES_WRITER_H */
