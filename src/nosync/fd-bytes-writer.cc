// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/fd-bytes-writer.h>
#include <nosync/fd-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/type-utils.h>
#include <utility>

using std::enable_shared_from_this;
using std::experimental::string_view;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::unique_ptr;


namespace nosync
{

namespace
{

class async_write_job : public enable_shared_from_this<async_write_job>
{
public:
    static void start(fd_watcher &watcher, shared_fd &&fd, string &&data, result_handler<void> &&res_handler);

    async_write_job(shared_fd &&fd, string &&data, result_handler<void> &&res_handler);

private:
    void handle_write_chunk();

    shared_fd fd;
    string data;
    size_t data_offset;
    result_handler<void> res_handler;
    unique_ptr<activity_handle> write_watch_handle;
};


class fd_bytes_writer : public bytes_writer, public enable_shared_from_this<fd_bytes_writer>
{
public:
    fd_bytes_writer(fd_watcher &watcher, shared_fd &&fd);

    void write_bytes(string &&data, result_handler<void> &&res_handler) override;

private:
    fd_watcher &watcher;
    shared_fd fd;
};


void async_write_job::start(fd_watcher &watcher, shared_fd &&fd, string &&data, result_handler<void> &&res_handler)
{
    auto fd_num = *fd;
    auto job = make_shared<async_write_job>(move(fd), move(data), move(res_handler));
    job->write_watch_handle = watcher.add_watch(
        fd_num, fd_watch_mode::output,
        [job]() {
            job->handle_write_chunk();
        });
}


async_write_job::async_write_job(
    shared_fd &&fd, string &&data, result_handler<void> &&res_handler)
    : fd(move(fd)), data(move(data)), data_offset(0U), res_handler(move(res_handler))
{
}


void async_write_job::handle_write_chunk()
{
    auto write_result = write_some_bytes_to_fd(*fd, string_view(data).substr(data_offset));
    if (write_result.is_ok()) {
        data_offset += write_result.get_value();
        if (data_offset >= data.size()) {
            if (write_watch_handle->is_enabled()) {
                write_watch_handle->disable();
            }
            res_handler(make_ok_result());
        }
    } else {
        if (write_watch_handle->is_enabled()) {
            write_watch_handle->disable();
        }
        res_handler(raw_error_result(write_result));
    }
}


fd_bytes_writer::fd_bytes_writer(fd_watcher &watcher, shared_fd &&fd)
    : watcher(watcher), fd(move(fd))
{
}


void fd_bytes_writer::write_bytes(
    string &&data, result_handler<void> &&res_handler)
{
    async_write_job::start(watcher, make_copy(fd), move(data), move(res_handler));
}

}


shared_ptr<bytes_writer> make_fd_bytes_writer(fd_watcher &watcher, shared_fd &&fd)
{
    return make_shared<fd_bytes_writer>(watcher, move(fd));
}

}
