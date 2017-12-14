// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/hex-encoded-messages-request-handler.h>
#include <nosync/string-utils.h>
#include <nosync/transforming-request-handler.h>
#include <stdexcept>
#include <system_error>
#include <utility>

using std::errc;
using std::invalid_argument;
using std::move;
using std::shared_ptr;
using std::string;


namespace nosync
{

shared_ptr<request_handler<string, string>> make_hex_encoded_messages_request_handler(
    event_loop &evloop, shared_ptr<request_handler<string, string>> &&raw_messages_req_handler)
{
    return make_full_transforming_request_handler<string, string, string, string>(
        evloop, move(raw_messages_req_handler),
        [](auto msg) {
            return make_ok_result(bytes_to_hex_string(msg));
        },
        [](auto msg) {
            try {
               return make_ok_result(bytes_from_hex_string(msg));
            } catch (const invalid_argument &) {
               return make_error_result<string>(errc::bad_message);
            }
        });
}

}
