#include "parsing.hpp"
#include <sstream>

std::string parse_request(const std::string& req, const std::string& start_token) {
    std::size_t start = req.find(start_token);
    if (start != std::string::npos) {
        start += start_token.length();
        std::size_t end = req.find("\r\n", start);
        return req.substr(start, end - start);
    }
    return "";
}


void parse_request_line(const std::string& request, std::string& method, std::string& path, std::string& protocol) {
    std::istringstream stream(request);
    stream >> method >> path >> protocol;
}

bool should_compress(const std::string& request) {
    std::string compression_method = parse_request(request, "Accept-Encoding: ");
    return compression_method.find("gzip") != std::string::npos;
}

bool should_close(const std::string& request) {
    return parse_request(request, "Connection: ") == "close";
}


int extract_int_header(const std::string& request, const std::string& key) {
    std::string val = parse_request(request, key);
    if (!val.empty() && val.back() == '\r') val.pop_back();
    return std::stoi(val);
}
