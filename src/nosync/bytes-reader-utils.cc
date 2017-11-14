// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <nosync/bytes-reader-utils.h>
#include <nosync/raw-error-result.h>
#include <system_error>
#include <tuple>
#include <utility>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using std::errc;
using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

constexpr auto default_read_size = 4096U;

constexpr auto buffer_prealloc_max_size = 4096U;


class read_fully_context
{
public:
    read_fully_context(
        shared_ptr<bytes_reader> &&reader, size_t size,
        result_handler<string> &&res_handler);

    static void continue_work(shared_ptr<read_fully_context> &&ctx);

private:
    shared_ptr<bytes_reader> reader;
    size_t size;
    result_handler<string> res_handler;
    string data;
};


read_fully_context::read_fully_context(
    shared_ptr<bytes_reader> &&reader, size_t size,
    result_handler<string> &&res_handler)
    : reader(move(reader)), size(size), res_handler(move(res_handler)), data()
{
    data.reserve(std::min<size_t>(size, buffer_prealloc_max_size));
}


void read_fully_context::continue_work(shared_ptr<read_fully_context> &&ctx)
{
    read_some_bytes(
        ctx->reader, ctx->size - ctx->data.size(),
        [ctx](auto read_res) mutable {
            if (!read_res.is_ok()) {
                ctx->res_handler(read_res);
                return;
            }

            ctx->data.append(read_res.get_value());

            if (ctx->data.size() >= ctx->size) {
                ctx->res_handler(make_ok_result(move(ctx->data)));
            } else if (read_res.get_value().empty()) {
                ctx->res_handler(raw_error_result(errc::no_message_available));
            } else {
                continue_work(move(ctx));
            }
        });
}

}


void read_some_bytes(
    const shared_ptr<bytes_reader> &reader, size_t max_size,
    result_handler<string> &&res_handler)
{
    reader->read_some_bytes(max_size, ch::nanoseconds::max(), move(res_handler));
}


void read_all_pending_bytes(
    shared_ptr<bytes_reader> &&reader, function<void(string)> &&data_consumer,
    function<void()> &&after_func)
{
    auto reader_ptr = reader.get();
    reader_ptr->read_some_bytes(
        default_read_size, 0ns,
        [reader = move(reader), data_consumer = move(data_consumer), after_func = move(after_func)](auto res) mutable {
            if (res.is_ok()) {
                data_consumer(move(res.get_value()));
                read_all_pending_bytes(move(reader), move(data_consumer), move(after_func));
            } else {
                after_func();
            }
        });
}


void read_bytes_fully(
    shared_ptr<bytes_reader> &&reader, size_t size,
    result_handler<string> &&res_handler)
{
    read_fully_context::continue_work(
        make_shared<read_fully_context>(move(reader), size, move(res_handler)));
}

}
