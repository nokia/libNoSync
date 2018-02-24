// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <experimental/array>
#include <functional>
#include <nosync/function-utils.h>
#include <nosync/time-utils.h>
#include <tuple>

using namespace std::chrono_literals;
using std::array;
using std::experimental::make_array;
using std::get;
using std::make_tuple;
using std::size_t;
using std::snprintf;
using std::string;
using std::strlen;
using std::tuple;
using std::uint32_t;


namespace nosync
{

namespace ch = std::chrono;

namespace
{

constexpr auto micros_dec_digits = 6U;


tm get_localtime_tm(ch::time_point<ch::system_clock> time)
{
    tm time_tm = {};
    auto time_time_t = ch::system_clock::to_time_t(time);
    localtime_r(&time_time_t, &time_tm);

    return time_tm;
}


tm get_gmtime_tm(ch::time_point<ch::system_clock> time)
{
    tm time_tm = {};
    auto time_time_t = ch::system_clock::to_time_t(time);
    gmtime_r(&time_time_t, &time_tm);

    return time_tm;
}


template<size_t buf_size>
array<char, buf_size> format_time(
    const tm &time_tm, const char (&format_template)[buf_size], const char *format)
{
    array<char, sizeof(format_template)> time_str_buf = {};
    ::strftime(time_str_buf.data(), time_str_buf.size(), format, &time_tm);

    return time_str_buf;
}


array<char, micros_dec_digits + 1> format_time_microseconds(ch::time_point<ch::system_clock> time)
{
    array<char, micros_dec_digits + 1> micros_str_buf = {};
    const auto micros_value = static_cast<uint32_t>(
        ch::duration_cast<ch::microseconds>(time.time_since_epoch() % 1s).count());
    snprintf(micros_str_buf.data(), micros_str_buf.size(), "%" PRIu32, micros_value);

    const auto micros_value_dec_digits = strlen(micros_str_buf.data());
    if (micros_value_dec_digits != micros_dec_digits) {
        std::copy_backward(
            micros_str_buf.begin(), micros_str_buf.begin() + micros_value_dec_digits,
            micros_str_buf.begin() + micros_dec_digits);
        std::fill(micros_str_buf.begin(), micros_str_buf.begin() + micros_dec_digits - micros_value_dec_digits, '0');
    }

    return micros_str_buf;
}


template<typename T, size_t N, size_t RN>
constexpr void concat_array_ranges(array<T, N> &output, const array<tuple<const T *, const T *>, RN> &ranges)
{
    auto out_it = output.begin();
    for (const auto &range : ranges) {
        out_it = std::copy(get<0>(range), get<1>(range), out_it);
    }
}


template<typename ...A>
constexpr auto concat_array_str_buf_arrays(const A ...str_buf)
{
    constexpr auto str_bufs_length_sum = reduce<size_t>(
        0, std::plus<std::size_t>(), (str_buf.size() - 1)...);
    array<char, str_bufs_length_sum + 1> concat_buffer = {};

    constexpr array<char, 1> null_terminator = {'\0'};

    concat_array_ranges(
        concat_buffer,
        make_array(
            make_tuple(str_buf.data(), str_buf.data() + str_buf.size() - 1)...,
            make_tuple(null_terminator.data(), null_terminator.data() + 1)));

    return concat_buffer;
}

}


ch::nanoseconds make_duration_from_timespec(const ::timespec &ts) noexcept
{
    return ch::seconds(ts.tv_sec) + ch::nanoseconds(ts.tv_nsec);
}


ch::microseconds make_duration_from_timeval(const ::timeval &tv) noexcept
{
    return ch::seconds(tv.tv_sec) + ch::microseconds(tv.tv_usec);
}


array<char, sizeof("YYYY-MM-DDTHH:MM:SS+HHMM")> format_time_to_localtime_seconds_tz_array(
    ch::time_point<ch::system_clock> time) noexcept
{
    return format_time(get_localtime_tm(time), "YYYY-MM-DDTHH:MM:SS+HHMM", "%Y-%m-%dT%H:%M:%S%z");
}


array<char, sizeof("YYYY-MM-DDTHH:MM:SS.123456+HHMM")> format_time_to_localtime_microseconds_tz_array(
    ch::time_point<ch::system_clock> time) noexcept
{
    const auto time_tm = get_localtime_tm(time);
    constexpr array<char, 2> dec_sep_str_buf = {"."};

    return concat_array_str_buf_arrays(
        format_time(time_tm, "YYYY-MM-DDTHH:MM:SS", "%Y-%m-%dT%H:%M:%S"),
        dec_sep_str_buf, format_time_microseconds(time),
        format_time(time_tm, "+HHMM", "%z"));
}


array<char, sizeof("YYYY-MM-DDTHH:MM:SS")> format_time_to_gmtime_seconds_array(
    ch::time_point<ch::system_clock> time) noexcept
{
    return format_time(get_gmtime_tm(time), "YYYY-MM-DDTHH:MM:SS", "%Y-%m-%dT%H:%M:%S");
}


array<char, sizeof("YYYY-MM-DDTHH:MM:SS.123456")> format_time_to_gmtime_microseconds_array(
    ch::time_point<ch::system_clock> time) noexcept
{
    constexpr array<char, 2> dec_sep_str_buf = {"."};

    return concat_array_str_buf_arrays(
        format_time(get_gmtime_tm(time), "YYYY-MM-DDTHH:MM:SS", "%Y-%m-%dT%H:%M:%S"),
        dec_sep_str_buf, format_time_microseconds(time));
}


string format_time_to_localtime_seconds_tz(ch::time_point<ch::system_clock> time)
{
    const auto time_str_buf = format_time_to_localtime_seconds_tz_array(time);
    return string(time_str_buf.data(), time_str_buf.size() - 1);
}


string format_time_to_localtime_microseconds_tz(ch::time_point<ch::system_clock> time)
{
    const auto time_str_buf = format_time_to_localtime_microseconds_tz_array(time);
    return string(time_str_buf.data(), time_str_buf.size() - 1);
}


string format_time_to_gmtime_seconds(ch::time_point<ch::system_clock> time)
{
    const auto time_str_buf = format_time_to_gmtime_seconds_array(time);
    return string(time_str_buf.data(), time_str_buf.size() - 1);
}


string format_time_to_gmtime_microseconds(ch::time_point<ch::system_clock> time)
{
    const auto time_str_buf = format_time_to_gmtime_microseconds_array(time);
    return string(time_str_buf.data(), time_str_buf.size() - 1);
}

}
