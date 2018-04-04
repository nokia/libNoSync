// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__RESULT_UTILS_H
#define NOSYNC__RESULT_UTILS_H

#include <nosync/raw-error-result.h>
#include <nosync/result.h>


namespace nosync
{

raw_error_result make_raw_error_result_from_errno();

raw_error_result make_timeout_raw_error_result();

}

#endif /* NOSYNC__RESULT_UTILS_H */
