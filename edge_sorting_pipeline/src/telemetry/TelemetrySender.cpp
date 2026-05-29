#include "telemetry/TelemetrySender.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

TelemetrySender::TelemetrySender(const std::string& host, int port)
    : host_(host), port_(port) {}

bool TelemetrySender::send(const TelemetryPayload& payload) {
    std::ostringstream json_stream;
    json_stream << "{\"material\": \"" << payload.material 
                << "\", \"confidence\": " << payload.confidence 
                << ", \"sorted_count\": " << payload.sorted_count << "}";
    std::string body = json_stream.str();

    std::ostringstream http_stream;
    http_stream << "POST /telemetry HTTP/1.1\r\n"
                << "Host: " << host_ << ":" << port_ << "\r\n"
                << "Content-Type: application/json\r\n"
                << "Content-Length: " << body.length() << "\r\n"
                << "Connection: close\r\n\r\n"
                << body;
    std::string request = http_stream.str();

    struct addrinfo hints, *res = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::string port_str = std::to_string(port_);
    if (getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return false;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        return false;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        close(sock);
        freeaddrinfo(res);
        return false;
    }

    freeaddrinfo(res);

    ssize_t bytes_written = write(sock, request.c_str(), request.length());
    if (bytes_written < 0) {
        close(sock);
        return false;
    }

    char buffer[256];
    while (read(sock, buffer, sizeof(buffer) - 1) > 0) {
        // Discard response
    }

    close(sock);
    return true;
}
