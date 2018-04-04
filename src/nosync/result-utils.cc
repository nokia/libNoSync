// This file is part of libnosync library. See LICENSE file for license details.
#include <cerrno>
#include <nosync/result-utils.h>
#include <system_error>

using std::errc;
using std::error_code;
using std::make_error_code;


namespace nosync
{

raw_error_result make_raw_error_result_from_errno()
{
    const auto errno_value = errno;
    return raw_error_result(error_code(errno_value, std::generic_category()));
}


raw_error_result make_timeout_raw_error_result()
{
    return raw_error_result(make_error_code(errc::timed_out));
}

}
