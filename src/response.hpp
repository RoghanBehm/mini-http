#include <string>
#include <sstream>
void send_response(std::ostringstream& rep, const std::string& body, std::string content_type, bool compress, bool close_c);