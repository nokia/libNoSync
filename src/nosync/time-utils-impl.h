// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__TIME_UTILS_IMPL_H
#define NOSYNC__TIME_UTILS_IMPL_H


namespace nosync
{

namespace time_utils_impl
{

template<typename SubsecDuration, typename Rep, typename Period>
constexpr std::tuple<long, long> make_sec_subsec_from_duration(const std::chrono::duration<Rep, Period> &duration) noexcept
{
    namespace ch = std::chrono;

    const auto duration_subsec = ch::duration_cast<SubsecDuration>(duration);
    const auto duration_sec = ch::duration_cast<ch::seconds>(duration_subsec);

    return {
        duration_sec.count(),
        SubsecDuration(duration_subsec - duration_sec).count(),
    };
}

}


template<typename Rep, typename Period>
constexpr ::timespec make_timespec_from_duration(const std::chrono::duration<Rep, Period> &duration) noexcept
{
    const auto sec_nsec = time_utils_impl::make_sec_subsec_from_duration<std::chrono::nanoseconds>(duration);
    return {std::get<0>(sec_nsec), std::get<1>(sec_nsec)};
}


template<typename Rep, typename Period>
constexpr ::timeval make_timeval_from_duration(const std::chrono::duration<Rep, Period> &duration) noexcept
{
    const auto sec_usec = time_utils_impl::make_sec_subsec_from_duration<std::chrono::microseconds>(duration);
    return {std::get<0>(sec_usec), std::get<1>(sec_usec)};
}


template<typename Rep, typename Period>
constexpr double to_float_seconds(const std::chrono::duration<Rep, Period> &duration) noexcept
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
}


template<class Clock, class Duration>
std::chrono::time_point<Clock> time_point_sat_add(std::chrono::time_point<Clock> time, Duration delta)
{
    using time_point = std::chrono::time_point<Clock>;
    constexpr auto zero_time = time_point(Duration::zero());

    time_point sat_sum;
    if (time >= zero_time && delta > (time_point::max() - time)) {
        sat_sum = time_point::max();
    } else if (time < zero_time && delta < (time_point::min() - time)) {
        sat_sum = time_point::min();
    } else {
        sat_sum = time + delta;
    }

    return sat_sum;
}

}

#endif /* NOSYNC__TIME_UTILS_IMPL_H */
