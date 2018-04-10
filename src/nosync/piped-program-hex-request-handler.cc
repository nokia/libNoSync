// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/hex-encoded-messages-request-handler.h>
#include <nosync/io-lines-request-handler.h>
#include <nosync/noop-func.h>
#include <nosync/piped-command.h>
#include <nosync/piped-program-hex-request-handler.h>
#include <utility>

using std::get;
using std::shared_ptr;
using std::string;
using std::vector;


namespace nosync
{

shared_ptr<request_handler<string, string>> start_piped_program_hex_request_handler(
    fd_watching_event_loop &evloop, const shared_ptr<request_handler<pid_t, int>> &subproc_reaper,
    const string &cmd_path, const vector<string> &cmd_args)
{
    auto cmd_info = start_piped_command(evloop, subproc_reaper, cmd_path, cmd_args, noop_func());
    auto &proc_io = get<shared_ptr<bytes_io>>(cmd_info);

    return make_hex_encoded_messages_request_handler(
        evloop, make_io_lines_request_handler(evloop, proc_io));
}

}
