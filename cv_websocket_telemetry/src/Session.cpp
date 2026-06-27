#include "Session.h"

Session::Session(tcp::socket socket) : ws_(std::move(socket)) {}

void Session::start(std::function<void()> on_ready) {
    ws_.async_accept(
        [self = shared_from_this(), cb = std::move(on_ready)](beast::error_code ec) mutable {
            self->onAccept(ec, std::move(cb));
        });
}

void Session::onAccept(beast::error_code ec, std::function<void()> on_ready) {
    if (ec) return;
    if (on_ready) on_ready();
    doRead();
}

void Session::doRead() {
    ws_.async_read(buf_,
        [self = shared_from_this()](beast::error_code ec, std::size_t bytes) {
            self->onRead(ec, bytes);
        });
}

void Session::onRead(beast::error_code ec, std::size_t) {
    if (ec == websocket::error::closed) return;
    if (ec) return;
    buf_.consume(buf_.size());
    doRead();
}

void Session::sendJson(const std::string& payload) {
    auto self = shared_from_this();
    auto msg  = std::make_shared<std::string>(payload);
    net::post(ws_.get_executor(), [self, msg]() {
        self->ws_.async_write(net::buffer(*msg),
            [self, msg](beast::error_code, std::size_t) {});
    });
}
