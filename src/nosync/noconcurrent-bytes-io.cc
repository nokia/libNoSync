// This file is part of libnosync library. See LICENSE file for license details.
#include <nosync/noconcurrent-bytes-io.h>
#include <stdexcept>
#include <utility>

namespace ch = std::chrono;
using std::enable_shared_from_this;
using std::make_shared;
using std::move;
using std::runtime_error;
using std::shared_ptr;
using std::size_t;
using std::string;


namespace nosync
{

namespace
{

class noconcurrent_bytes_io : public bytes_io, public enable_shared_from_this<noconcurrent_bytes_io>
{
public:
    explicit noconcurrent_bytes_io(shared_ptr<bytes_io> &&base_bio);

    void read_some_bytes(
        size_t max_size, ch::nanoseconds timeout,
        result_handler<string> &&res_handler) override;
    void write_bytes(
        string &&data, result_handler<void> &&res_handler) override;

private:
    shared_ptr<bytes_io> base_bio;
    bool bio_active;
};


noconcurrent_bytes_io::noconcurrent_bytes_io(shared_ptr<bytes_io> &&base_bio)
    : base_bio(move(base_bio)), bio_active(false)
{
}


void noconcurrent_bytes_io::read_some_bytes(
    size_t max_size, ch::nanoseconds timeout, result_handler<string> &&res_handler)
{
    if (bio_active) {
        throw runtime_error("async bytes read on sequential bio with operation in progress");
    }

    bio_active = true;
    base_bio->read_some_bytes(
        max_size, timeout,
        [bio = shared_from_this(), res_handler = move(res_handler)](auto res) {
            bio->bio_active = false;
            res_handler(move(res));
        });
}


void noconcurrent_bytes_io::write_bytes(
    string &&data, result_handler<void> &&res_handler)
{
    if (bio_active) {
        throw runtime_error("async bytes write on sequential bio with operation in progress");
    }

    bio_active = true;
    base_bio->write_bytes(
        move(data),
        [bio = shared_from_this(), res_handler = move(res_handler)](auto res) {
            bio->bio_active = false;
            res_handler(move(res));
        });
}

}


shared_ptr<bytes_io> make_noconcurrent_bytes_io(shared_ptr<bytes_io> &&base_bio)
{
    return make_shared<noconcurrent_bytes_io>(move(base_bio));
}

}
