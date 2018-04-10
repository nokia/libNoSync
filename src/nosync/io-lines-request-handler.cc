// This file is part of libnosync library. See LICENSE file for license details.
#include <experimental/optional>
#include <experimental/string_view>
#include <nosync/input-messages-dispatch-handler.h>
#include <nosync/io-lines-request-handler.h>
#include <nosync/lines-reader.h>
#include <nosync/raw-error-result.h>
#include <nosync/result-handler-utils.h>
#include <nosync/sequential-chunks-writer.h>
#include <nosync/string-utils.h>
#include <nosync/time-utils.h>

namespace ch = std::chrono;
using namespace std::string_literals;
using std::errc;
using std::experimental::nullopt;
using std::experimental::optional;
using std::experimental::string_view;
using std::make_shared;
using std::move;
using std::nullptr_t;
using std::shared_ptr;
using std::string;
using std::uint64_t;


namespace nosync
{

namespace
{

constexpr auto req_id_separator = ':';


struct request_context
{
    explicit request_context(result_handler<string> &&res_handler);

    void call_res_handler(result<string> &&res);

    result_handler<string> res_handler;
    optional<result<void>> saved_req_res;
    optional<result<string>> saved_resp_res;
};


request_context::request_context(result_handler<string> &&res_handler)
    : res_handler(move(res_handler)), saved_req_res(), saved_resp_res()
{
}


void request_context::call_res_handler(result<string> &&res)
{
    auto tmp_res_handler = move(res_handler);
    res_handler = nullptr;
    tmp_res_handler(move(res));
}


string encode_request_line(uint64_t req_id, string_view req_data)
{
    const auto req_id_hex_array = number_to_hex_digits_array(req_id);

    string req_line;
    req_line.reserve(req_id_hex_array.size() + 1 + req_data.size() + 1);

    req_line.append(req_id_hex_array.begin(), req_id_hex_array.end());
    req_line.push_back(req_id_separator);
    req_line.append(req_data.begin(), req_data.end());
    req_line.push_back('\n');

    return req_line;
}


string encode_request_id(uint64_t req_id)
{
    return make_string(number_to_hex_digits_array(req_id));
}


optional<string> try_decode_response_line_id(string_view resp_line)
{
    auto sep_pos = resp_line.find(req_id_separator);
    return sep_pos != string_view::npos
        ? std::experimental::make_optional(resp_line.substr(0, sep_pos).to_string())
        : nullopt;
}


string decode_response_line_data(string_view resp_line)
{
    auto sep_pos = resp_line.find(req_id_separator);
    return sep_pos != string_view::npos ? resp_line.substr(sep_pos + 1).to_string() : ""s;
}


class io_lines_request_handler : public request_handler<string, string>
{
public:
    io_lines_request_handler(event_loop &evloop, const shared_ptr<bytes_io> &lines_io);

    void handle_request(
        string &&data, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    event_loop &evloop;
    shared_ptr<request_handler<string, string>> resp_lines_dispatcher;
    shared_ptr<request_handler<string, void>> req_lines_writer;
    uint64_t next_req_id;
};


io_lines_request_handler::io_lines_request_handler(event_loop &evloop, const shared_ptr<bytes_io> &lines_io)
    : evloop(evloop),
    resp_lines_dispatcher(
        make_input_messages_dispatch_handler(
            evloop, make_lines_reader(evloop, lines_io), try_decode_response_line_id)),
    req_lines_writer(make_sequential_chunks_writer(evloop, lines_io)),
    next_req_id(0)
{
}


void io_lines_request_handler::handle_request(
    string &&data, ch::nanoseconds timeout,
    result_handler<string> &&res_handler)
{
    if (data.find('\n') != string::npos) {
        invoke_result_handler_later(evloop, move(res_handler), raw_error_result(errc::invalid_argument));
        return;
    }

    const auto req_id = next_req_id;
    ++next_req_id;

    auto ctx = make_shared<request_context>(move(res_handler));

    resp_lines_dispatcher->handle_request(
        encode_request_id(req_id), timeout,
        [ctx](auto resp_res) {
            auto resp_data = make_ok_result(decode_response_line_data(resp_res.get_value()));
            if (ctx->saved_req_res) {
                ctx->call_res_handler(move(resp_data));
            } else {
                ctx->saved_resp_res = move(resp_data);
            }
        });

    req_lines_writer->handle_request(
        encode_request_line(req_id, data), timeout,
        [ctx](auto req_res) {
            if (req_res.is_ok()) {
                if (ctx->saved_resp_res) {
                    ctx->call_res_handler(move(*ctx->saved_resp_res));
                } else {
                    ctx->saved_req_res = move(req_res);
                }
            } else {
                ctx->call_res_handler(raw_error_result(req_res));
            }
        });
}

}


shared_ptr<request_handler<string, string>> make_io_lines_request_handler(
    event_loop &evloop, const shared_ptr<bytes_io> &lines_io)
{
    return make_shared<io_lines_request_handler>(evloop, lines_io);
}

}
