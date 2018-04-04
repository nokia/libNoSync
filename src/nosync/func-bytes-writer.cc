// This file is part of libnosync library. See LICENSE file for license details.
#include <functional>
#include <memory>
#include <nosync/func-bytes-writer.h>
#include <string>
#include <utility>

using std::function;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;


namespace nosync
{

namespace
{

class func_bytes_writer : public bytes_writer
{
public:
    explicit func_bytes_writer(function<void(string &&, result_handler<void> &&)> &&writer_func);

    void write_bytes(string &&data, result_handler<void> &&res_handler) override;

private:
    function<void(string &&, result_handler<void> &&)> writer_func;
};


func_bytes_writer::func_bytes_writer(
    function<void(string &&, result_handler<void> &&)> &&writer_func)
    : writer_func(move(writer_func))
{
}


void func_bytes_writer::write_bytes(string &&data, result_handler<void> &&res_handler)
{
    writer_func(move(data), move(res_handler));
}

}


shared_ptr<bytes_writer> make_func_bytes_writer(
    function<void(string &&, result_handler<void> &&)> &&writer_func)
{
    return make_shared<func_bytes_writer>(move(writer_func));
}

}
