#include "TelemetryServer.h"
#include <thread>
#include <vector>

TelemetryServer::TelemetryServer(SessionManager& mgr, uint16_t port)
    : acceptor_(ioc_, {net::ip::tcp::v4(), port}), mgr_(mgr) {}

void TelemetryServer::run(int thread_count) {
    doAccept();
    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i)
        threads.emplace_back([this] { ioc_.run(); });
    for (auto& t : threads) t.join();
}

void TelemetryServer::stop() {
    net::post(ioc_, [this] { ioc_.stop(); });
}

void TelemetryServer::doAccept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto s = std::make_shared<Session>(std::move(socket));
                mgr_.add(s);
                s->start();
            }
            if (!ioc_.stopped()) doAccept();
        });
}
