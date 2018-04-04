// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__ECLOCK_H
#define NOSYNC__ECLOCK_H

#include <chrono>


namespace nosync
{

/*!
Clock to be used for time inside event loops.

Currently it provides the same time values as std::chrono::steady_clock but we
use it to ensure specific duration type and to detect accidental usage of
steady_clock::now() in the application code.
*/
class eclock
{
public:
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<eclock, duration>;

    static constexpr bool is_steady = true;

    time_point now() const;
};

}

#endif /* NOSYNC__ECLOCK_H */
