// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <array>
#include <chrono>
#include <experimental/filesystem>
#include <fcntl.h>
#include <iterator>
#include <nosync/event-loop.h>
#include <nosync/fd-bytes-reader.h>
#include <nosync/fd-bytes-writer.h>
#include <nosync/func-bytes-reader.h>
#include <nosync/func-bytes-writer.h>
#include <nosync/io-utils.h>
#include <nosync/owned-fd.h>
#include <nosync/piped-command.h>
#include <nosync/raw-error-result.h>
#include <nosync/reader-writer-bytes-io.h>
#include <nosync/result-handler-utils.h>
#include <nosync/shared-fd.h>
#include <nosync/type-utils.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

namespace ch = std::chrono;
using namespace std::string_literals;
using std::array;
using std::errc;
using std::experimental::filesystem::path;
using std::function;
using std::move;
using std::shared_ptr;
using std::string;
using std::tuple;
using std::vector;


namespace nosync
{

namespace
{

tuple<::pid_t, shared_ptr<bytes_io>> make_failed_start_command_result(event_loop &evloop)
{
    return {
        static_cast<::pid_t>(-1),
        make_reader_writer_bytes_io(
            make_func_bytes_reader(
                [&evloop](auto, auto, auto res_handler) {
                    invoke_result_handler_later(
                        evloop, move(res_handler), raw_error_result(errc::no_child_process));
                }),
            make_func_bytes_writer(
                [&evloop](auto, auto res_handler) {
                    invoke_result_handler_later(
                        evloop, move(res_handler), raw_error_result(errc::no_child_process));
                }))
    };
}


bool try_create_pipe(array<owned_fd, 2> &out_pipe_fds)
{
    int raw_pipe_fds[2];
    int pipe_retval = ::pipe2(&raw_pipe_fds[0], O_CLOEXEC);

    bool success = pipe_retval == 0;
    if (success) {
        out_pipe_fds[0] = owned_fd(raw_pipe_fds[0]);
        out_pipe_fds[1] = owned_fd(raw_pipe_fds[1]);
    }

    return success;
}


bool try_set_nonblocking(int fd)
{
    auto fcntl_res = ::fcntl(fd, F_GETFL);
    if (fcntl_res < 0) {
        return false;
    }

    bool success = ::fcntl(fd, F_SETFL, fcntl_res | O_NONBLOCK) >= 0;

    return success;
}

}


tuple<::pid_t, shared_ptr<bytes_io>> start_piped_command(
    fd_watching_event_loop &evloop, const shared_ptr<request_handler<::pid_t, int>> &subproc_reaper,
    const string &cmd_path, const vector<string> &cmd_args,
    function<void(int)> &&status_collector)
{
    array<owned_fd, 2> sub_stdin_fds;
    array<owned_fd, 2> sub_stdout_fds;
    if (!try_create_pipe(sub_stdin_fds) || !try_create_pipe(sub_stdout_fds)
        || !try_set_nonblocking(*sub_stdin_fds[1]) || !try_set_nonblocking(*sub_stdout_fds[0])) {
        return make_failed_start_command_result(evloop);
    }

    auto subproc_io = make_reader_writer_bytes_io(
        make_fd_bytes_reader(evloop, shared_fd(move(sub_stdout_fds[0]))),
        make_fd_bytes_writer(evloop, shared_fd(move(sub_stdin_fds[1]))));

    auto fork_res = fork();
    if (fork_res < 0) {
        return make_failed_start_command_result(evloop);
    };

    const auto cmd_filename = path(cmd_path).filename();

    vector<const char *> cmd_args_cstrs = {cmd_filename.c_str()};
    std::transform(
        cmd_args.begin(), cmd_args.end(), std::back_inserter(cmd_args_cstrs),
        std::mem_fn(&string::c_str));
    cmd_args_cstrs.push_back(nullptr);

    if (fork_res == 0) {
        // FIXME: handle source pipes on fd 0/1
        if (dup2(*sub_stdin_fds[0], STDIN_FILENO) >= 0 && dup2(*sub_stdout_fds[1], STDOUT_FILENO) >= 0) {
            ::execv(cmd_path.c_str(), const_cast<char **>(cmd_args_cstrs.data()));
        }
        _exit(0xFF);
    }

    sub_stdin_fds[0].reset();
    sub_stdout_fds[1].reset();

    subproc_reaper->handle_request(
        make_copy(fork_res), ch::nanoseconds::max(),
        [status_collector = move(status_collector)](auto res) {
            if (res.is_ok()) {
                status_collector(res.get_value());
            }
        });

    return {fork_res, move(subproc_io)};
}


tuple<::pid_t, shared_ptr<bytes_io>> start_piped_shell_command(
    fd_watching_event_loop &evloop, const shared_ptr<request_handler<pid_t, int>> &subproc_reaper,
    const string &shell_cmd, function<void(int)> &&status_collector)
{
    return start_piped_command(
        evloop, subproc_reaper, "/bin/sh"s, {"-c"s, shell_cmd}, move(status_collector));
}

}
