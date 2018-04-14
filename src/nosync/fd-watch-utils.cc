#include <functional>
#include <memory>
#include <nosync/event-loop-utils.h>
#include <nosync/fd-watch-utils.h>
#include <nosync/result-utils.h>
#include <utility>

namespace ch = std::chrono;
using std::make_shared;
using std::move;
using std::unique_ptr;


namespace nosync
{

namespace
{

struct timeouting_watch_context
{
    void handle_result(result<void> res);

    result_handler<void> res_handler;
    unique_ptr<activity_handle> watch_handle;
    unique_ptr<activity_handle> timeout_task_handle;
};


void timeouting_watch_context::handle_result(result<void> res)
{
    for (auto handle_ptr : {&watch_handle , &timeout_task_handle}) {
        auto &handle = *handle_ptr;
        if (handle) {
            if (handle->is_enabled()) {
                handle->disable();
            }
            handle.reset();
        }
    }

    res_handler(res);
}

}


void watch_fd_with_timeout(
    fd_watching_event_loop &evloop, int fd, fd_watch_mode mode, ch::nanoseconds timeout,
    result_handler<void> &&res_handler)
{
    auto ctx = make_shared<timeouting_watch_context>(
        timeouting_watch_context{move(res_handler), nullptr, nullptr});

    ctx->watch_handle = evloop.add_watch(
        fd, mode,
        [ctx]() {
            ctx->handle_result(make_ok_result());
        });
    ctx->timeout_task_handle = invoke_with_etime_delay(
        evloop, timeout,
        [ctx]() {
            ctx->handle_result(make_timeout_raw_error_result());
        });
}

}
