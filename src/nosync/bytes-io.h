// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__BYTES_IO_H
#define NOSYNC__BYTES_IO_H

#include <nosync/bytes-reader.h>
#include <nosync/bytes-writer.h>


namespace nosync
{

/*!
Interface "aggregating" bytes_reader and bytes_writer.

This interface inherits from both bytes_reader and bytes_writer without adding
any extra operations. It's intended to be implemented for bi-directional
communication channels.
*/
class bytes_io : public bytes_reader, public bytes_writer
{
};

}

#endif /* NOSYNC__BYTES_IO_H */
