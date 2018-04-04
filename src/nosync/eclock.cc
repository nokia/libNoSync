// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/eclock.h>

namespace ch = std::chrono;


namespace nosync
{

eclock::time_point eclock::now() const
{
    return time_point(ch::steady_clock::now().time_since_epoch());
}

}
