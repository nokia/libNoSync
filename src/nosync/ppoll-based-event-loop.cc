// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <cerrno>
#include <experimental/optional>
#include <nosync/eclock.h>
#include <nosync/manual-event-loop.h>
#include <nosync/manual-fd-watcher.h>
#include <nosync/ppoll-based-event-loop.h>
#include <nosync/time-utils.h>
#include <poll.h>
#include <vector>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using std::errc;
using std::error_code;
using std::experimental::optional;
using std::function;
using std::get;
using std::make_error_code;
using std::make_shared;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::system_category;
using std::unique_ptr;
using std::vector;


namespace nosync
{

namespace
{

int call_ppoll_with_abs_timeout(
    pollfd *fds, nfds_t nfds, const eclock &clock, optional<ch::time_point<eclock>> abs_timeout)
{
    int ppoll_retval;
    do {
        optional<timespec> ppoll_timeout;
        if (abs_timeout && *abs_timeout != ch::time_point<eclock>::max()) {
            ppoll_timeout = make_timespec_from_duration(std::max<ch::nanoseconds>(*abs_timeout - clock.now(), 0ns));
        }

        ppoll_retval = ::ppoll(fds, nfds, ppoll_timeout ? &*ppoll_timeout : nullptr, nullptr);
    } while (ppoll_retval == -1 && errno == EINTR);

    return ppoll_retval;
}


constexpr short map_watch_mode_to_poll_event(fd_watch_mode mode)
{
    return
        mode == fd_watch_mode::input
            ? POLLIN
            : mode == fd_watch_mode::output
                ? POLLOUT
                : 0;
}


vector<::pollfd> map_fd_watches_to_pollfds(manual_fd_watcher &fd_watcher)
{
    vector<::pollfd> pollfds;
    for (const auto &watch_key : fd_watcher.get_watches_fd_info()) {
        pollfds.push_back(
            ::pollfd({
                get<int>(watch_key),
                map_watch_mode_to_poll_event(get<fd_watch_mode>(watch_key)),
                0,
            })
        );
    }

    return pollfds;
}


bool notify_fd_watches_from_pollfd(manual_fd_watcher &fd_watcher, const ::pollfd &pollfd_item)
{
    if (pollfd_item.revents == 0) {
        return true;
    }

    bool normal_return = true;

    for (auto mode : {fd_watch_mode::input, fd_watch_mode::output}) {
        if ((pollfd_item.events & map_watch_mode_to_poll_event(mode)) == 0) {
            continue;
        }

        if (!fd_watcher.notify_watches(pollfd_item.fd, mode)) {
            normal_return = false;
            break;
        }
    }

    return normal_return;
}


bool notify_fd_watches_from_pollfds(manual_fd_watcher &fd_watcher, const vector<::pollfd> &pollfd_items)
{
    return std::all_of(
        pollfd_items.begin(), pollfd_items.end(),
        [&](const auto &pollfd_item) {
            return notify_fd_watches_from_pollfd(fd_watcher, pollfd_item);
        });
}


class ppoll_based_event_loop : public full_fd_watching_event_loop
{
public:
    ppoll_based_event_loop();

    unique_ptr<activity_handle> invoke_at(ch::time_point<eclock> time, function<void()> &&task) override;
    ch::time_point<eclock> get_etime() const override;

    unique_ptr<activity_handle> add_watch(int fd, fd_watch_mode mode, function<void()> &&notify_func) override;

    error_code run_iterations() override;
    void quit() override;

private:
    eclock clock;
    unique_ptr<manual_event_loop> sub_evloop;
    unique_ptr<manual_fd_watcher> sub_fd_watcher;
    ch::time_point<eclock> etime;
    bool quit_request_pending;
};


ppoll_based_event_loop::ppoll_based_event_loop()
    : clock(), sub_evloop(make_unique<manual_event_loop>(clock.now())),
    sub_fd_watcher(make_unique<manual_fd_watcher>()), quit_request_pending(false)
{
    etime = sub_evloop->get_etime();
}


unique_ptr<activity_handle> ppoll_based_event_loop::invoke_at(ch::time_point<eclock> time, function<void()> &&task)
{
    return sub_evloop->invoke_at(time, move(task));
}


ch::time_point<eclock> ppoll_based_event_loop::get_etime() const
{
    return etime;
}


unique_ptr<activity_handle> ppoll_based_event_loop::add_watch(int fd, fd_watch_mode mode, function<void()> &&notify_func)
{
    return sub_fd_watcher->add_watch(fd, mode, move(notify_func));
}


error_code ppoll_based_event_loop::run_iterations()
{
    error_code errc;

    while (!errc) {
        bool all_tasks_processed = sub_evloop->process_time_passage(
            std::max<ch::nanoseconds>(etime - sub_evloop->get_etime(), 0ns));
        if (!all_tasks_processed) {
            errc = make_error_code(errc::interrupted);
            break;
        }

        auto pollfd_items = map_fd_watches_to_pollfds(*sub_fd_watcher);
        auto next_task_time = sub_evloop->get_earliest_task_time();

        if (!next_task_time && pollfd_items.empty()) {
            break;
        }

        int ppoll_retval = call_ppoll_with_abs_timeout(
            pollfd_items.data(), pollfd_items.size(), clock, next_task_time);

        etime = clock.now();

        if (ppoll_retval > 0) {
            if (!notify_fd_watches_from_pollfds(*sub_fd_watcher, pollfd_items)) {
                errc = make_error_code(errc::interrupted);
                break;
            }
        } else if (ppoll_retval < 0) {
            errc = error_code(errno, system_category());
            break;
        }

        if (quit_request_pending) {
            errc = make_error_code(errc::interrupted);
            break;
        }
    }

    return errc;
}


void ppoll_based_event_loop::quit()
{
    sub_evloop->quit();
    sub_fd_watcher->quit();
    quit_request_pending = true;
}

}


shared_ptr<full_fd_watching_event_loop> make_ppoll_based_event_loop()
{
    return make_shared<ppoll_based_event_loop>();
}

}
