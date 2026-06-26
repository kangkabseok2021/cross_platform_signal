#pragma once
#include "SessionManager.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdint>

class TelemetryServer {
public:
    TelemetryServer(SessionManager& mgr, uint16_t port);
    void run(int thread_count = 2);
    void stop();

private:
    net::io_context ioc_;
    tcp::acceptor   acceptor_;
    SessionManager& mgr_;

    void doAccept();
};
