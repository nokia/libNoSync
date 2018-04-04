// This file is part of libnosync library. See LICENSE file for license details.
#include <ctime>
#include <gtest/gtest.h>
#include <nosync/time-utils.h>
#include <sys/time.h>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using nosync::make_duration_from_timespec;
using nosync::make_duration_from_timeval;
using nosync::make_timespec_from_duration;
using nosync::make_timeval_from_duration;
using nosync::time_point_sat_add;
using nosync::to_float_seconds;


TEST(NosyncTimeUtils, MakeTimespecFromDuration)
{
    timespec ts1 = make_timespec_from_duration(987s + 150'000'000ns);
    ASSERT_EQ(ts1.tv_sec, 987);
    ASSERT_EQ(ts1.tv_nsec, 150'000'000);

    timespec ts2 = make_timespec_from_duration(-987s - 150'000'000ns);
    ASSERT_EQ(ts2.tv_sec, -987);
    ASSERT_EQ(ts2.tv_nsec, -150'000'000);
}


TEST(NosyncTimeUtils, MakeTimeDurationFromTimespec)
{
    ASSERT_EQ(make_duration_from_timespec(timespec{987, 999'000'000}), 987s + 999'000'000ns);
    ASSERT_EQ(make_duration_from_timespec(timespec{-987, -999'000'000}), -987s - 999'000'000ns);
}


TEST(NosyncTimeUtils, MakeTimeDurationFromTimespecFromDuration)
{
    constexpr auto duration1 = 987s + 999'000'000ns;
    ASSERT_EQ(make_duration_from_timespec(make_timespec_from_duration(duration1)), duration1);

    constexpr auto duration2 = -987s - 999'000'000ns;
    ASSERT_EQ(make_duration_from_timespec(make_timespec_from_duration(duration2)), duration2);
}


TEST(NosyncTimeUtils, MakeTimevalFromDuration)
{
    timeval tv1 = make_timeval_from_duration(987s + 350'000us);
    ASSERT_EQ(tv1.tv_sec, 987);
    ASSERT_EQ(tv1.tv_usec, 350'000);

    timeval tv2 = make_timeval_from_duration(-987s - 350'000us);
    ASSERT_EQ(tv2.tv_sec, -987);
    ASSERT_EQ(tv2.tv_usec, -350'000);
}


TEST(NosyncTimeUtils, MakeTimeDurationFromTimeval)
{
    ASSERT_EQ(make_duration_from_timeval(timeval{987, 999'000}), 987s + 999'000us);
    ASSERT_EQ(make_duration_from_timeval(timeval{-987, -999'000}), -987s - 999'000us);
}


TEST(NosyncTimeUtils, MakeTimeDurationFromTimevalFromDuration)
{
    constexpr auto duration1 = 987s + 999'000us;
    ASSERT_EQ(make_duration_from_timeval(make_timeval_from_duration(duration1)), duration1);
    constexpr auto duration2 = -987s - 999'000us;
    ASSERT_EQ(make_duration_from_timeval(make_timeval_from_duration(duration2)), duration2);
}


TEST(NosyncTimeUtils, ToFloatSeconds)
{
    EXPECT_FLOAT_EQ(to_float_seconds(ch::duration<double>(5.0)), 5.0);

    EXPECT_FLOAT_EQ(to_float_seconds(1s), 1.0);
    EXPECT_FLOAT_EQ(to_float_seconds(-3s), -3.0);

    EXPECT_FLOAT_EQ(to_float_seconds(250ms), 0.25);
    EXPECT_FLOAT_EQ(to_float_seconds(-500ms), -0.5);
}


TEST(NosyncTimeUtils, TestTimePointSatAddBasic)
{
    using time_point = ch::time_point<ch::steady_clock>;

    ASSERT_EQ(time_point_sat_add(time_point::max(), 0ns), time_point::max());
    ASSERT_EQ(time_point_sat_add(time_point::max() - 20s, 10s), time_point::max() - 10s);
    ASSERT_EQ(time_point_sat_add(time_point(1000s), 10s), time_point(1010s));

    ASSERT_EQ(time_point_sat_add(time_point::min(), 0ns), time_point::min());
    ASSERT_EQ(time_point_sat_add(time_point::min() + 20s, -10s), time_point::min() + 10s);
    ASSERT_EQ(time_point_sat_add(time_point(-1000s), -10s), time_point(-1010s));

    ASSERT_LT(time_point_sat_add(time_point::max(), -1ns), time_point::max());
    ASSERT_GT(time_point_sat_add(time_point::min(), 1ns), time_point::min());
}


TEST(NosyncTimeUtils, TestTimePointSatAddWithOverflow)
{
    using time_point = ch::time_point<ch::steady_clock>;

    ASSERT_EQ(time_point_sat_add(time_point::max(), 1ns), time_point::max());
    ASSERT_EQ(time_point_sat_add(time_point::max() - 20s, 30s), time_point::max());
    ASSERT_EQ(time_point_sat_add(time_point(1000s), ch::nanoseconds::max()), time_point::max());
    ASSERT_EQ(time_point_sat_add(time_point::max(), ch::nanoseconds::max()), time_point::max());
}


TEST(NosyncTimeUtils, TestTimePointSatAddWithUnderflow)
{
    using time_point = ch::time_point<ch::steady_clock>;

    ASSERT_EQ(time_point_sat_add(time_point::min(), -1ns), time_point::min());
    ASSERT_EQ(time_point_sat_add(time_point::min() + 20s, -30s), time_point::min());
    ASSERT_EQ(time_point_sat_add(time_point(-1000s), ch::nanoseconds::min()), time_point::min());
    ASSERT_EQ(time_point_sat_add(time_point::min(), ch::nanoseconds::min()), time_point::min());
}
