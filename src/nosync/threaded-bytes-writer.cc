// This file is part of libnosync library. See LICENSE file for license details.
#include <chrono>
#include <nosync/func-bytes-writer.h>
#include <nosync/threaded-bytes-writer.h>
#include <nosync/threaded-request-handler.h>
#include <utility>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using std::function;
using std::move;
using std::shared_ptr;
using std::string;


namespace nosync
{

shared_ptr<bytes_writer> make_threaded_bytes_writer(
    function<void(function<void()>)> &&evloop_mt_executor,
    function<void(function<void()>)> &&thread_executor,
    function<result<void>(string)> &&sync_writer)
{
    auto write_req_handler = make_threaded_request_handler<string, void>(
        move(evloop_mt_executor), move(thread_executor),
        [sync_writer = move(sync_writer)](auto request, auto) {
            return sync_writer(move(request));
        });

    return make_func_bytes_writer(
        [write_req_handler = move(write_req_handler)](auto data, auto res_handler) {
            write_req_handler->handle_request(move(data), ch::nanoseconds::max(), move(res_handler));
        }
    );
}

}
