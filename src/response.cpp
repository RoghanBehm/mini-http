#include "gzip.hpp"
#include <sstream>
#include <vector>

void send_response(std::ostringstream& rep, const std::string& body, std::string content_type, bool compress, bool close_c) {
    std::vector<unsigned char> compressed;
    size_t length = 0;

    if (compress) {
        size_t max_size = compressBound(body.size()) + 18;
        compressed.resize(max_size);
        gzip_compress(body.data(), body.size(), compressed.data(), &max_size);
        compressed.resize(max_size);
        length = compressed.size();
    } else {
        length = body.size();
    }

    rep << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: " << content_type << "\r\n";
    if (close_c) rep << "Connection: close\r\n";
    if (compress) rep << "Content-Encoding: gzip\r\n";
    rep << "Content-Length: " << length << "\r\n\r\n";

    if (compress)
        rep.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    else
        rep << body;
}