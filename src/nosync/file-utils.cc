// This file is part of libnosync library. See LICENSE file for license details.
#include <fcntl.h>
#include <nosync/file-utils.h>
#include <nosync/result-utils.h>
#include <sys/stat.h>
#include <sys/types.h>

using std::string;


namespace nosync
{

namespace
{

result<owned_fd> open_file(const string &path, int flags)
{
    int open_retval = open(path.c_str(), flags | O_NOCTTY | O_CLOEXEC | O_NONBLOCK);
    return open_retval >= 0
        ? make_ok_result(owned_fd(open_retval))
        : make_raw_error_result_from_errno();
}

}


result<owned_fd> open_file_for_read(const string &path)
{
    return open_file(path, O_RDONLY);
}


result<owned_fd> open_file_for_write(const string &path)
{
    return open_file(path, O_WRONLY);
}

}
