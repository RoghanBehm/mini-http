#include "gzip.hpp"

void gzip_compress(const char *input, size_t inputLength, unsigned char *output, size_t *outputLength) {
    z_stream stream = {0};
    deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input));
    stream.avail_in = inputLength;

    stream.next_out = output;
    stream.avail_out = *outputLength;

    deflate(&stream, Z_FINISH);
    *outputLength = stream.total_out;

    deflateEnd(&stream);
}