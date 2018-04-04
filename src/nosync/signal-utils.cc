// This file is part of libnosync library. See LICENSE file for license details.
#include <csignal>
#include <nosync/exceptions.h>
#include <nosync/signal-utils.h>


namespace nosync
{

void block_signal(int signal_num)
{
    sigset_t sigset;
    if (::sigemptyset(&sigset) < 0 || ::sigaddset(&sigset, signal_num) < 0) {
        throw_system_error_from_errno();
    }

    if (sigprocmask(SIG_BLOCK, &sigset, nullptr) < 0) {
        throw_system_error_from_errno();
    }
}


void ignore_signal(int signal_num)
{
    ::signal(signal_num, SIG_IGN);
}

}
