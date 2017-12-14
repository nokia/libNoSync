// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/sequential-chunks-writer.h>
#include <nosync/sequential-request-handler.h>

namespace ch = std::chrono;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;


namespace nosync
{

namespace
{

class chunks_writer : public request_handler<string, void>
{
public:
    explicit chunks_writer(shared_ptr<bytes_writer> &&base_writer);

    void handle_request(string &&data, ch::nanoseconds timeout, result_handler<void> &&res_handler) override;

private:
    shared_ptr<bytes_writer> base_writer;
};


chunks_writer::chunks_writer(shared_ptr<bytes_writer> &&base_writer)
    : base_writer(move(base_writer))
{
}


void chunks_writer::handle_request(string &&data, ch::nanoseconds, result_handler<void> &&res_handler)
{
    base_writer->write_bytes(move(data), move(res_handler));
}

}


shared_ptr<request_handler<string, void>> make_sequential_chunks_writer(
    event_loop &evloop, shared_ptr<bytes_writer> &&base_writer)
{
    return make_sequential_request_handler<string, void>(
        evloop, make_shared<chunks_writer>(move(base_writer)));
}

}
