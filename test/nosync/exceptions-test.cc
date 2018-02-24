// This file is part of libnosync library. See LICENSE file for license details.
#include <cerrno>
#include <cstring>
#include <exception>
#include <gtest/gtest.h>
#include <nosync/exceptions.h>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std::string_literals;
using nosync::get_nonnull_or_throw;
using nosync::print_current_exception_info;
using nosync::print_exception_info;
using nosync::throw_system_error_from_errno;
using std::exception_ptr;
using std::invalid_argument;
using std::make_exception_ptr;
using std::nested_exception;
using std::regex;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::system_error;
using std::throw_with_nested;


TEST(NosyncExceptions, ThrowSystemErrorFromErrnoWithoutMessage) {
    constexpr auto error_code = ENOENT;
    errno = error_code;
    try {
        throw_system_error_from_errno();
        FAIL();
    } catch (system_error &e) {
        ASSERT_EQ(e.code().value(), error_code);
    }
}


TEST(NosyncExceptions, ThrowSystemErrorFromErrnoWithMessage) {
    const auto error_msg = "TEST-MSG";
    constexpr auto error_code = EINVAL;
    errno = error_code;
    try {
        throw_system_error_from_errno(string(error_msg));
        FAIL();
    } catch (system_error &e) {
        ASSERT_EQ(e.code().value(), error_code);
        ASSERT_TRUE(::strstr(e.what(), error_msg) != nullptr);
    }
}


TEST(NosyncExceptions, PrintExceptionInfoNone) {
    stringstream ostr;
    print_exception_info(exception_ptr(), ostr, "TEST-PREFIX:"s);
    ASSERT_EQ(ostr.str(), ""s);
}


TEST(NosyncExceptions, PrintCurrentExceptionInfoNone) {
    stringstream ostr;
    print_current_exception_info(ostr, "TEST-PREFIX:"s);
    ASSERT_EQ(ostr.str(), ""s);
}


TEST(NosyncExceptions, PrintExceptionInfoSimple) {
    stringstream ostr;
    print_exception_info(make_exception_ptr(invalid_argument("TEST-MSG")), ostr, "TEST-PREFIX:"s);
    ASSERT_TRUE(regex_match(ostr.str(), regex("TEST-PREFIX:.*TEST-MSG.*\n")));
}


TEST(NosyncExceptions, PrintCurrentExceptionInfoSimple) {
    stringstream ostr;
    try {
        throw invalid_argument("TEST-MSG");
    } catch (invalid_argument &) {
        print_current_exception_info(ostr, "TEST-PREFIX:"s);
    }
    ASSERT_TRUE(regex_match(ostr.str(), regex("TEST-PREFIX:.*TEST-MSG.*\n")));
}


TEST(NosyncExceptions, PrintExceptionInfoNested) {
    exception_ptr eptr;
    try {
        try {
            try {
                throw "TEST-MSG-0";
            } catch (...) {
                throw_with_nested(runtime_error("TEST-MSG-1"));
            }
        } catch (runtime_error &) {
            throw_with_nested(runtime_error("TEST-MSG-2"));
        }
    } catch (runtime_error &e) {
        eptr = std::current_exception();
    }

    stringstream ostr;
    print_exception_info(eptr, ostr, "TEST-PREFIX:"s);
    ASSERT_TRUE(regex_match(ostr.str(), regex("TEST-PREFIX:.*TEST-MSG-2.*\nTEST-PREFIX:.*TEST-MSG-1.*\nTEST-PREFIX:.*\n")));
}


TEST(NosyncExceptions, PrintCurrentExceptionInfoNested) {
    stringstream ostr;
    try {
        try {
            try {
                throw "TEST-MSG-0";
            } catch (...) {
                throw_with_nested(runtime_error("TEST-MSG-1"));
            }
        } catch (runtime_error &) {
            throw_with_nested(runtime_error("TEST-MSG-2"));
        }
    } catch (runtime_error &) {
        print_current_exception_info(ostr, "TEST-PREFIX:"s);
    }

    ASSERT_TRUE(regex_match(ostr.str(), regex("TEST-PREFIX:.*TEST-MSG-2.*\nTEST-PREFIX:.*TEST-MSG-1.*\nTEST-PREFIX:.*\n")));
}


TEST(NosyncExceptions, PrintExceptionInfoNestedNull) {
    stringstream ostr;
    print_exception_info(make_exception_ptr(nested_exception()), ostr, "TEST-PREFIX:"s);
    ASSERT_TRUE(regex_match(ostr.str(), regex("TEST-PREFIX:.*\n")));
}


TEST(NosyncExceptions, PrintCurrentExceptionInfoNestedNull) {
    stringstream ostr;
    try {
        throw nested_exception();
    } catch (nested_exception &) {
        print_current_exception_info(ostr, "TEST-PREFIX:"s);
    }

    ASSERT_TRUE(regex_match(ostr.str(), regex("TEST-PREFIX:.*\n")));
}


TEST(NosyncExceptions, GetNonnullOrThrowWithNotNull) {
    auto *test_pointer = "abc";
    ASSERT_EQ(get_nonnull_or_throw(test_pointer, "unused message"s), test_pointer);
}


TEST(NosyncExceptions, GetNonnullOrThrowWithNull) {
    const auto missing_data_message = "missing data"s;
    try {
        get_nonnull_or_throw(static_cast<const char *>(nullptr), missing_data_message);
        FAIL();
    } catch (runtime_error &e) {
        auto *error_msg = e.what();
        ASSERT_NE(error_msg, nullptr);
        ASSERT_NE(string(error_msg).find(missing_data_message), string::npos);
    }
}
