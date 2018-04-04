// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <nosync/const-bytes-reader.h>
#include <nosync/result-handler-utils.h>
#include <utility>

namespace ch = std::chrono;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

class const_bytes_reader : public bytes_reader
{
public:
    const_bytes_reader(event_loop &evloop, string &&input_bytes);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    event_loop &evloop;
    string input_bytes;
    size_t offset;
};


const_bytes_reader::const_bytes_reader(event_loop &evloop, string &&input_bytes)
    : evloop(evloop), input_bytes(move(input_bytes)), offset(0)
{
}


void const_bytes_reader::read_some_bytes(
    size_t max_size, ch::nanoseconds, result_handler<string> &&res_handler)
{
    auto chunk = input_bytes.substr(offset, max_size);
    offset += chunk.size();
    invoke_result_handler_later(evloop, move(res_handler), make_ok_result(move(chunk)));
}

}


shared_ptr<bytes_reader> make_const_bytes_reader(event_loop &evloop, string &&input_bytes)
{
    return make_shared<const_bytes_reader>(evloop, move(input_bytes));
}

}
