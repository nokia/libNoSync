// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TIME_UTILS_H
#define NOSYNC__TIME_UTILS_H

#include <array>
#include <chrono>
#include <ctime>
#include <string>
#include <tuple>


namespace nosync
{

constexpr std::chrono::seconds day_secs = std::chrono::hours(24);
constexpr std::chrono::seconds week_secs = day_secs * 7;

template<typename Rep, typename Period>
constexpr ::timespec make_timespec_from_duration(const std::chrono::duration<Rep, Period> &duration) noexcept;

std::chrono::nanoseconds make_duration_from_timespec(const ::timespec &ts) noexcept;

template<typename Rep, typename Period>
constexpr ::timeval make_timeval_from_duration(const std::chrono::duration<Rep, Period> &duration) noexcept;

std::chrono::microseconds make_duration_from_timeval(const ::timeval &tv) noexcept;

template<typename Rep, typename Period>
constexpr double to_float_seconds(const std::chrono::duration<Rep, Period> &duration) noexcept;

std::array<char, sizeof("YYYY-MM-DDTHH:MM:SS+HHMM")> format_time_to_localtime_seconds_tz_array(
    std::chrono::time_point<std::chrono::system_clock> time) noexcept;
std::array<char, sizeof("YYYY-MM-DDTHH:MM:SS.123456+HHMM")> format_time_to_localtime_microseconds_tz_array(
    std::chrono::time_point<std::chrono::system_clock> time) noexcept;

std::array<char, sizeof("YYYY-MM-DDTHH:MM:SS")> format_time_to_gmtime_seconds_array(
    std::chrono::time_point<std::chrono::system_clock> time) noexcept;
std::array<char, sizeof("YYYY-MM-DDTHH:MM:SS.123456")> format_time_to_gmtime_microseconds_array(
    std::chrono::time_point<std::chrono::system_clock> time) noexcept;

std::string format_time_to_localtime_seconds_tz(std::chrono::time_point<std::chrono::system_clock> time);
std::string format_time_to_localtime_microseconds_tz(std::chrono::time_point<std::chrono::system_clock> time);

std::string format_time_to_gmtime_seconds(std::chrono::time_point<std::chrono::system_clock> time);
std::string format_time_to_gmtime_microseconds(std::chrono::time_point<std::chrono::system_clock> time);

template<class Clock, class Duration>
std::chrono::time_point<Clock> time_point_sat_add(std::chrono::time_point<Clock> time, Duration delta);

}

#include <nosync/time-utils-impl.h>

#endif /* NOSYNC__TIME_UTILS_H */
