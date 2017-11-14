// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/func-bytes-reader.h>

namespace ch = std::chrono;
using std::function;
using std::make_shared;
using std::shared_ptr;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

class func_bytes_reader : public bytes_reader
{
public:
    explicit func_bytes_reader(function<void(size_t, ch::nanoseconds, result_handler<string>)> &&reader_func);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;

private:
    function<void(size_t, ch::nanoseconds, result_handler<string>)> reader_func;
};


func_bytes_reader::func_bytes_reader(
    function<void(size_t, ch::nanoseconds, result_handler<string>)> &&reader_func)
    : reader_func(move(reader_func))
{
}


void func_bytes_reader::read_some_bytes(
        size_t max_size, ch::nanoseconds timeout, result_handler<string> &&res_handler)
{
    reader_func(max_size, timeout, move(res_handler));
}

}


shared_ptr<bytes_reader> make_func_bytes_reader(
    function<void(size_t, ch::nanoseconds, result_handler<string>)> &&reader_func)
{
    return make_shared<func_bytes_reader>(move(reader_func));
}

}
