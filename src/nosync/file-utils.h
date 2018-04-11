// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__FILE_UTILS_H
#define NOSYNC__FILE_UTILS_H

#include <nosync/owned-fd.h>
#include <nosync/result.h>
#include <string>


namespace nosync
{

result<owned_fd> open_file_for_read(const std::string &path);
result<owned_fd> open_file_for_write(const std::string &path);

}

#endif /* NOSYNC__FILE_UTILS_H */
