// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/reader-writer-bytes-io.h>
#include <utility>

namespace ch = std::chrono;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;


namespace nosync
{

namespace
{

class reader_writer_bytes_io : public bytes_io
{
public:
    reader_writer_bytes_io(shared_ptr<bytes_reader> reader, shared_ptr<bytes_writer> writer);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;
    void write_bytes(
        string &&data, result_handler<void> &&res_handler) override;

private:
    shared_ptr<bytes_reader> reader;
    shared_ptr<bytes_writer> writer;
};


reader_writer_bytes_io::reader_writer_bytes_io(
    shared_ptr<bytes_reader> reader, shared_ptr<bytes_writer> writer)
    : reader(move(reader)), writer(move(writer))
{
}


void reader_writer_bytes_io::read_some_bytes(
    size_t max_size, ch::nanoseconds timeout, result_handler<string> &&res_handler)
{
    reader->read_some_bytes(max_size, timeout, move(res_handler));
}


void reader_writer_bytes_io::write_bytes(
    string &&data, result_handler<void> &&res_handler)
{
    writer->write_bytes(move(data), move(res_handler));
}

}


shared_ptr<bytes_io> make_reader_writer_bytes_io(
    shared_ptr<bytes_reader> reader, shared_ptr<bytes_writer> writer)
{
    return make_shared<reader_writer_bytes_io>(move(reader), move(writer));
}

}
