// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__HEX_ENCODED_MESSAGES_REQUEST_HANDLER_H
#define NOSYNC__HEX_ENCODED_MESSAGES_REQUEST_HANDLER_H

#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<request_handler<std::string, std::string>> make_hex_encoded_messages_request_handler(
    event_loop &evloop, std::shared_ptr<request_handler<std::string, std::string>> &&raw_messages_req_handler);

}

#endif /* NOSYNC__HEX_ENCODED_MESSAGES_REQUEST_HANDLER_H */
