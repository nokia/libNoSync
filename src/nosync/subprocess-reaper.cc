// This file is part of libnosync library. See LICENSE file for license details.
#include <csignal>
#include <map>
#include <nosync/event-loop-utils.h>
#include <nosync/memory-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/result-utils.h>
#include <nosync/signalfd-watcher.h>
#include <nosync/subprocess-reaper.h>
#include <nosync/time-utils.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <vector>

namespace ch = std::chrono;
using std::enable_shared_from_this;
using std::errc;
using std::make_shared;
using std::map;
using std::move;
using std::runtime_error;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;


namespace nosync
{

namespace
{

struct waitpid_request
{
    unique_ptr<activity_handle> timeout_task_handle;
    result_handler<int> res_handler;
};


class subprocess_reaper : public request_handler<pid_t, int>, public enable_shared_from_this<subprocess_reaper>
{
public:
    explicit subprocess_reaper(fd_watching_event_loop &evloop);
    ~subprocess_reaper() override;

    void start();

    void handle_request(
        pid_t &&req_pid, ch::nanoseconds timeout,
        result_handler<int> &&res_handler) override;

private:
    void collect_terminated_children();

    fd_watching_event_loop &evloop;
    map<pid_t, waitpid_request> pending_requests;
    shared_ptr<interface_type> sigchld_watcher;
};


subprocess_reaper::subprocess_reaper(fd_watching_event_loop &evloop)
    : evloop(evloop), pending_requests(), sigchld_watcher()
{
}


subprocess_reaper::~subprocess_reaper()
{
    if (!pending_requests.empty()) {
        vector<result_handler<int>> pending_res_handlers;
        pending_res_handlers.reserve(pending_requests.size());
        for (auto &req : pending_requests) {
            pending_res_handlers.push_back(move(req.second.res_handler));
        }

        invoke_later(
            evloop,
            [pending_res_handlers = move(pending_res_handlers)]() mutable {
                for (auto &res_handler : pending_res_handlers) {
                    res_handler(raw_error_result(errc::operation_canceled));
                    res_handler = nullptr;
                }
            });
    }
}


void subprocess_reaper::start()
{
    if (sigchld_watcher) {
        throw runtime_error("subprocess reaper already started");
    }

    sigchld_watcher = make_signalfd_watcher(
        evloop, SIGCHLD,
        make_weak_this_func_proxy(
            this,
            [](auto &self) {
                self.collect_terminated_children();
            }));
}


void subprocess_reaper::collect_terminated_children()
{
    while (true) {
        int wstatus;
        auto waitpid_res = ::waitpid(-1, &wstatus, WNOHANG);
        if (waitpid_res < 0) {
            break;
        }

        auto req_iter = pending_requests.find(waitpid_res);
        if (req_iter != pending_requests.end()) {
            if (req_iter->second.timeout_task_handle) {
                req_iter->second.timeout_task_handle->disable();
            }
            req_iter->second.res_handler(make_ok_result(wstatus));
        }
    }
}


void subprocess_reaper::handle_request(
    pid_t &&req_pid, ch::nanoseconds timeout,
    result_handler<int> &&res_handler)
{
    unique_ptr<activity_handle> timeout_task_handle;
    if (timeout != ch::nanoseconds::max()) {
        timeout_task_handle = evloop.invoke_at(
            time_point_sat_add(evloop.get_etime(), timeout),
            [req_pid, this]() {
                auto req_iter = pending_requests.find(req_pid);
                if (req_iter != pending_requests.end()) {
                    req_iter->second.res_handler(make_timeout_raw_error_result());
                    pending_requests.erase(req_iter);
                }
            });
    }

    pending_requests.emplace(
        req_pid,
        waitpid_request{
            move(timeout_task_handle),
            move(res_handler)
        });
}

}


shared_ptr<request_handler<pid_t, int>> make_subprocess_reaper(fd_watching_event_loop &evloop)
{
    auto reaper = make_shared<subprocess_reaper>(evloop);
    reaper->start();
    return reaper;
}

}
