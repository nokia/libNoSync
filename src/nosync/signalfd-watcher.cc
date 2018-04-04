// This file is part of libnosync library. See LICENSE file for license details.
#include <csignal>
#include <cstring>
#include <nosync/bytes-reader-utils.h>
#include <nosync/exceptions.h>
#include <nosync/fd-bytes-reader.h>
#include <nosync/memory-utils.h>
#include <nosync/shared-fd.h>
#include <nosync/signalfd-watcher.h>
#include <nosync/type-utils.h>
#include <sys/signalfd.h>
#include <system_error>

using std::enable_shared_from_this;
using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::system_error;


namespace nosync
{

namespace
{

shared_fd create_signalfd_for_signal(int signal_num)
{
    sigset_t sigset;
    if (sigemptyset(&sigset) < 0 || sigaddset(&sigset, signal_num) < 0) {
        throw_system_error_from_errno();
    }

    int signal_fd = ::signalfd(-1, &sigset, SFD_NONBLOCK | SFD_CLOEXEC);
    if (signal_fd < 0) {
        throw_system_error_from_errno();
    }

    return shared_fd(signal_fd);
}


class signalfd_watcher : public interface_type, public enable_shared_from_this<signalfd_watcher>
{
public:
    signalfd_watcher(fd_watching_event_loop &evloop, shared_fd &&signal_fd, function<void()> &&signal_handler);

    void continue_watching();

private:
    function<void()> signal_handler;
    shared_fd signal_fd;
    shared_ptr<bytes_reader> siginfo_reader;
};


signalfd_watcher::signalfd_watcher(
    fd_watching_event_loop &evloop, shared_fd &&signal_fd, function<void()> &&signal_handler)
    : signal_handler(move(signal_handler)), signal_fd(signal_fd),
    siginfo_reader(make_fd_bytes_reader(evloop, make_copy(signal_fd)))
{
}


void signalfd_watcher::continue_watching()
{
    read_bytes_fully(
        make_copy(siginfo_reader), sizeof(signalfd_siginfo),
        [watcher_wptr = weak_from_that(this)](auto read_res) {
            if (!read_res.is_ok()) {
                throw system_error(read_res.get_error(), "reading signalfd_siginfo failed");
            }

            auto watcher_ptr = watcher_wptr.lock();
            if (watcher_ptr) {
                watcher_ptr->signal_handler();
                watcher_ptr->continue_watching();
            }
        });
}

}


shared_ptr<interface_type> make_signalfd_watcher(
    fd_watching_event_loop &evloop, int signal_num, function<void()> &&signal_handler)
{
    auto watcher = make_shared<signalfd_watcher>(
        evloop, create_signalfd_for_signal(signal_num), move(signal_handler));
    watcher->continue_watching();

    return watcher;
}

}
