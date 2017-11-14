// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <deque>
#include <functional>
#include <nosync/buffered-bytes-reader.h>
#include <nosync/memory-utils.h>
#include <string>
#include <utility>

namespace ch = std::chrono;
using namespace std::chrono_literals;
using std::deque;
using std::enable_shared_from_this;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

class buffered_bytes_reader : public bytes_reader, public enable_shared_from_this<buffered_bytes_reader>
{
public:
    buffered_bytes_reader(size_t buffer_size, shared_ptr<bytes_reader> &&base_reader);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    size_t max_buffer_size;
    shared_ptr<bytes_reader> base_reader;
    deque<char> buffer;
};


buffered_bytes_reader::buffered_bytes_reader(
    size_t buffer_size, shared_ptr<bytes_reader> &&base_reader)
    : max_buffer_size(buffer_size), base_reader(move(base_reader))
{
}


void buffered_bytes_reader::read_some_bytes(
    size_t max_size, ch::nanoseconds timeout,
    result_handler<string> &&res_handler)
{
    if (!buffer.empty()) {
        const auto chunk_size = std::min(buffer.size(), max_size);
        auto chunk = string(buffer.cbegin(), buffer.cbegin() + chunk_size);
        buffer.erase(buffer.begin(), buffer.begin() + chunk_size);
        base_reader->read_some_bytes(
            0U, ch::nanoseconds::max(),
            [res_handler = move(res_handler), chunk = move(chunk)](auto) mutable {
                res_handler(make_ok_result(move(chunk)));
            });
    } else if (max_size >= max_buffer_size) {
        base_reader->read_some_bytes(max_size, timeout, move(res_handler));
    } else {
        base_reader->read_some_bytes(
            max_buffer_size, timeout,
            [reader_wptr = weak_from_that(this), max_size, res_handler = move(res_handler)](auto res) {
                if (!res.is_ok() || res.get_value().size() <= max_size) {
                    res_handler(move(res));
                } else {
                    auto &new_data = res.get_value();
                    const auto chunk_size = max_size;

                    auto reader_ptr = reader_wptr.lock();
                    if (reader_ptr) {
                        reader_ptr->buffer.insert(
                            reader_ptr->buffer.end(), new_data.begin() + chunk_size, new_data.end());
                    }

                    res_handler(make_ok_result(new_data.substr(0, chunk_size)));
                }
            });
    }
}

}


shared_ptr<bytes_reader> make_buffered_bytes_reader(
    size_t buffer_size, shared_ptr<bytes_reader> &&base_reader)
{
    return make_shared<buffered_bytes_reader>(buffer_size, move(base_reader));
}

}
