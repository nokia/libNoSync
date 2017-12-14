// This file is part of libnosync library. See LICENSE file for license details.
#ifndef NOSYNC__INPUT_MESSAGES_DISPATCH_HANDLER_H
#define NOSYNC__INPUT_MESSAGES_DISPATCH_HANDLER_H

#include <experimental/optional>
#include <experimental/string_view>
#include <nosync/bytes-reader.h>
#include <nosync/event-loop.h>
#include <nosync/request-handler.h>
#include <memory>
#include <string>


namespace nosync
{

std::shared_ptr<request_handler<std::string, std::string>> make_input_messages_dispatch_handler(
    event_loop &evloop, std::shared_ptr<request_handler<std::nullptr_t, std::string>> &&messages_reader,
    std::function<std::experimental::optional<std::string>(std::experimental::string_view)> &&message_id_decoder);

}

#endif /* NOSYNC__INPUT_MESSAGES_DISPATCH_HANDLER_H */
