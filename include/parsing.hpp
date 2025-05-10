#include <string>



std::string parse_request(const std::string& req, const std::string& start_token);
void parse_request_line(const std::string& request, std::string& method, std::string& path, std::string& protocol);
bool should_compress(const std::string& request);
bool should_close(const std::string& request);
int extract_int_header(const std::string& request, const std::string& key);