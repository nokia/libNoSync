// This file is part of libnosync library. See LICENSE file for license details.
#include <algorithm>
#include <nosync/function-utils.h>
#include <type_traits>
#include <utility>

using std::function;
using std::move;
using std::size_t;
using std::string;


namespace nosync
{

function<string(size_t)> make_buffering_bytes_supplier(function<string()> &&base_supplier)
{
    string bytes_buffer;
    size_t bytes_buffer_offset = 0;

    return [base_supplier = move(base_supplier), bytes_buffer, bytes_buffer_offset](size_t chunk_size) mutable {
        string output;

        while (true) {
            const auto sub_chunk_size = std::min(
                chunk_size - output.size(),
                bytes_buffer.size() - bytes_buffer_offset);
            output.append(bytes_buffer, bytes_buffer_offset, sub_chunk_size);
            bytes_buffer_offset += sub_chunk_size;

            if (output.size() == chunk_size) {
                break;
            }

            bytes_buffer = base_supplier();
            bytes_buffer_offset = 0;

            if (bytes_buffer.empty()) {
                break;
            }
        }

        return output;
    };
}

}
