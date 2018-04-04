// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__NOCONCURRENT_BYTES_IO_H
#define NOSYNC__NOCONCURRENT_BYTES_IO_H

#include <nosync/bytes-io.h>
#include <memory>


namespace nosync
{

std::shared_ptr<bytes_io> make_noconcurrent_bytes_io(std::shared_ptr<bytes_io> &&base_bio);

}

#endif /* NOSYNC__NOCONCURRENT_BYTES_IO_H */
