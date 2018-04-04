// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__EXCEPTIONS_H
#define NOSYNC__EXCEPTIONS_H

#include <exception>
#include <iostream>
#include <string>


namespace nosync
{

[[noreturn]] void throw_system_error_from_errno();
[[noreturn]] void throw_system_error_from_errno(const std::string &msg);

void print_exception_info(const std::exception_ptr &eptr, std::ostream &output, const std::string &line_prefix = {});
void print_current_exception_info(std::ostream &output, const std::string &line_prefix = {});

template<typename T>
T *get_nonnull_or_throw(T *ptr, const std::string &msg_for_null);

}

#include <nosync/exceptions-impl.h>

#endif /* NOSYNC__EXCEPTIONS_H */
