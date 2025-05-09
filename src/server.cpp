#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <filesystem> 
#include <fstream>
#include "gzip.hpp"
#include "threadpool.hpp"
#include "parsing.hpp"
#include "response.hpp"


std::string read_request(int client_fd) {
    char buffer[4096];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        return ""; // either closed or error
    }
    return std::string(buffer, bytes_received);
}


void handle_route(const std::string& method, const std::string& path, const std::string& request,
                  std::ostringstream& rep, const std::string& root_dir, bool compress, bool close_c) {
    std::string slug = path.substr(1);
    std::string first;
    std::size_t isolate = slug.find('/');

    if (isolate != std::string::npos) {
        first = slug.substr(0, isolate);
    } else {
        first = slug;
    }
    
    if (slug.empty()) {
        rep << "HTTP/1.1 200 OK\r\n\r\n";
    } else if (first == "echo") {
        std::string prefix = "echo/";
        std::string echo_body = slug.substr(prefix.length());
        send_response(rep, echo_body, "text/plain", compress, close_c);
    } else if (first == "user-agent") {
        std::string agent = parse_request(request, "User-Agent: ");
        send_response(rep, agent, "text/plain", compress, close_c);
    } else if (first == "files") {
        std::string filename = slug.substr(slug.find('/') + 1);
        std::filesystem::path full_path = std::filesystem::path(root_dir) / filename;

        if (method == "GET") {
            if (std::filesystem::exists(full_path)) {
                std::ifstream file(full_path, std::ios::binary);
                std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                send_response(rep, contents, "application/octet-stream", compress, close_c);
            }
        } else if (method == "POST") {
            int clength = extract_int_header(request, "Content-Length: ");
            std::string ctype = parse_request(request, "Content-Type: ");
            if (ctype == "application/octet-stream") {
                std::size_t head_end = request.find("\r\n\r\n");
                if (head_end != std::string::npos) {
                    std::string body = request.substr(head_end + 4);
                    std::ofstream file(full_path, std::ios::binary);
                    if (file) {
                        file.write(body.c_str(), clength);
                        rep << "HTTP/1.1 201 Created\r\n\r\n";
                    }
                }
            }
        }
    }

    if (rep.str().empty()) {
        rep << "HTTP/1.1 404 Not Found\r\n\r\n";
    }
}




void handle_client(int client_fd_, const std::string& root_dir) {
    while (true) {
        std::string request = read_request(client_fd_);
        if (request.empty()) {
            break; // client closed or error
        }

        std::ostringstream rep;
        std::string method, path, protocol;
        parse_request_line(request, method, path, protocol);

        bool compress = should_compress(request);
        bool close_c = should_close(request);

        handle_route(method, path, request, rep, root_dir, compress, close_c);

        std::string reply = rep.str();
        send(client_fd_, reply.c_str(), reply.size(), 0);

        if (close_c) break;
    }
    close(client_fd_);
}


int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;


  ThreadPool pool;
  std::string directory = "./";
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--directory" && i + 1 < argc) {
      directory = argv[++i];
    } else {
      std::cerr << "Unknown or incomplete option: " << arg << std::endl;
    }
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  

  

  while (true) {
    std::cout << "Waiting for a client to connect...\n";
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    std::cout << "Client connected\n";
    
    
    if (client_fd < 0) {
      // need to handle error here
      continue;
    }

    pool.enqueue([client_fd, directory]() {
      handle_client(client_fd, directory);
    });
    
  }
  
  
  close(server_fd);

  return 0;
}


