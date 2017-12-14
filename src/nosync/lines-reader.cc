// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <deque>
#include <nosync/event-loop-utils.h>
#include <nosync/lines-reader.h>
#include <nosync/memory-utils.h>
#include <nosync/raw-error-result.h>
#include <nosync/result-handler-utils.h>
#include <nosync/time-limited-bytes-reader.h>
#include <nosync/time-utils.h>
#include <nosync/type-utils.h>
#include <system_error>
#include <tuple>
#include <utility>

namespace ch = std::chrono;
using std::deque;
using std::enable_shared_from_this;
using std::errc;
using std::get;
using std::make_shared;
using std::make_tuple;
using std::move;
using std::nullptr_t;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::tuple;


namespace nosync
{

namespace
{

constexpr size_t input_chunk_size = 8192;


void read_line_impl(
    shared_ptr<bytes_reader> &&reader, size_t max_line_size, string &&data, result_handler<tuple<string, string>> &&res_handler)
{
    reader->read_some_bytes(
        std::min(max_line_size + 1 - data.size(), input_chunk_size),
        ch::nanoseconds::max(),
        merge_result_handler<string>(
            move(res_handler),
            [reader, max_line_size, data = move(data)](auto new_data, auto res_handler) mutable {
                if (new_data.empty()) {
                    res_handler(raw_error_result(errc::no_message_available));
                    return;
                }

                auto eol_pos = new_data.find('\n');

                if (eol_pos != string::npos) {
                    data.append(new_data, 0, eol_pos);
                    res_handler(make_ok_result(make_tuple(data, new_data.substr(eol_pos + 1))));
                } else if (data.size() + new_data.size() > max_line_size) {
                    res_handler(raw_error_result(errc::file_too_large));
                } else {
                    data.append(new_data);
                    read_line_impl(move(reader), max_line_size, move(data), move(res_handler));
                }
            }));
}


void read_line(shared_ptr<bytes_reader> &&reader, size_t max_line_size, result_handler<tuple<string, string>> &&res_handler)
{
    read_line_impl(move(reader), max_line_size, "", move(res_handler));
}


class lines_reader : public request_handler<nullptr_t, string>, public enable_shared_from_this<lines_reader>
{
public:
    lines_reader(event_loop &evloop, shared_ptr<bytes_reader> base_reader, size_t max_line_size);

    void handle_request(
        nullptr_t &&, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    event_loop &evloop;
    shared_ptr<bytes_reader> base_reader;
    size_t max_line_size;
    deque<char> buffer;
};


lines_reader::lines_reader(
    event_loop &evloop, shared_ptr<bytes_reader> base_reader, size_t max_line_size)
    : evloop(evloop), base_reader(move(base_reader)), max_line_size(max_line_size), buffer()
{
}


void lines_reader::handle_request(
    nullptr_t &&, ch::nanoseconds timeout,
    result_handler<string> &&res_handler)
{
    auto eol_iter = std::find(buffer.begin(), buffer.end(), '\n');
    if (eol_iter != buffer.end()) {
        string line_data(buffer.begin(), eol_iter);
        buffer.erase(buffer.begin(), eol_iter + 1);
        invoke_result_handler_later(evloop, move(res_handler), make_ok_result(move(line_data)));
    } else {
        string prefix(buffer.begin(), buffer.end());
        buffer.clear();
        const auto prefix_size = prefix.size();
        read_line(
            make_time_limited_bytes_reader(
                evloop, time_point_sat_add(evloop.get_etime(), timeout), make_copy(base_reader)),
            max_line_size - prefix_size,
            merge_result_handler<tuple<string, string>>(
                move(res_handler),
                [req_handler_wptr = weak_from_that(this), prefix = move(prefix)](auto read_pair, auto res_handler) {
                    const auto &line_reminder = get<0>(read_pair);
                    const auto &after_line = get<1>(read_pair);
                    auto req_handler_ptr = req_handler_wptr.lock();
                    if (req_handler_ptr) {
                        req_handler_ptr->buffer.insert(req_handler_ptr->buffer.end(), after_line.begin(), after_line.end());
                    }
                    res_handler(make_ok_result(prefix + line_reminder));
                }));
    }
}

}


shared_ptr<request_handler<nullptr_t, string>> make_lines_reader(
    event_loop &evloop, shared_ptr<bytes_reader> base_reader, size_t max_line_size)
{
    return make_shared<lines_reader>(evloop, move(base_reader), max_line_size);
}

}
